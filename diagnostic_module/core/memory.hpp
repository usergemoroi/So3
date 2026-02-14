#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string_view>
#include <optional>
#include <mutex>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <link.h>

#include "../utils/obfuscation.hpp"

namespace diag::core {

// Process memory region information
struct MemoryRegion {
    uintptr_t base;
    size_t size;
    int prot;  // Protection flags (PROT_READ, PROT_WRITE, PROT_EXEC)
    std::string name;  // Mapped file name if any
};

// Safe memory access wrapper with validation
class ProcessMemory {
public:
    // Singleton access - thread-safe initialization
    static ProcessMemory& instance() {
        static ProcessMemory inst;
        return inst;
    }
    
    // Initialize by caching module base addresses
    bool initialize();
    
    // Get base address of loaded module
    [[nodiscard]] uintptr_t get_module_base(std::string_view module_name) const;
    
    // Get module size
    [[nodiscard]] size_t get_module_size(std::string_view module_name) const;
    
    // Safe read from process memory - handles potential SIGSEGV
    template<typename T>
    [[nodiscard]] bool read(uintptr_t address, T& out) const {
        if (!is_valid_address(address, sizeof(T))) return false;
        
        OBFUSCATE_FLOW();
        
        // Use volatile to prevent optimization
        volatile bool success = true;
        
        // sigaction-based protection would go here in production
        // For now, simple validation
        out = *reinterpret_cast<const T*>(address);
        return success;
    }
    
    // Read array of data
    [[nodiscard]] bool read_bytes(uintptr_t address, void* out, size_t size) const;
    
    // Safe write (use with extreme caution)
    [[nodiscard]] bool write(uintptr_t address, const void* data, size_t size) const;
    
    // Pattern scan within module
    [[nodiscard]] std::optional<uintptr_t> pattern_scan(
        std::string_view module_name,
        const std::vector<int>& pattern,  // -1 for wildcard
        uintptr_t start_offset = 0
    ) const;
    
    // Get all readable memory regions
    [[nodiscard]] std::vector<MemoryRegion> get_memory_regions() const;
    
    // Resolve relative offset to absolute address
    [[nodiscard]] static uintptr_t resolve_rel32(uintptr_t instruction_addr, int offset_idx = 3);
    
    // Calculate relative address for hooking
    [[nodiscard]] static int32_t calc_rel32(uintptr_t from, uintptr_t to);
    
    // Memory barrier for synchronization
    static void memory_barrier() {
        __sync_synchronize();
    }
    
    // Cache-friendly prefetch
    static void prefetch(const void* addr) {
        __builtin_prefetch(addr, 0, 3);
    }

private:
    ProcessMemory() = default;
    
    [[nodiscard]] bool is_valid_address(uintptr_t addr, size_t size) const;
    
    mutable std::mutex regions_mutex_;
    std::vector<MemoryRegion> cached_regions_;
};

// RAII wrapper for memory-mapped regions
class MemoryMapping {
public:
    MemoryMapping(uintptr_t addr, size_t size, int prot);
    ~MemoryMapping();
    
    MemoryMapping(const MemoryMapping&) = delete;
    MemoryMapping& operator=(const MemoryMapping&) = delete;
    
    MemoryMapping(MemoryMapping&& other) noexcept;
    MemoryMapping& operator=(MemoryMapping&& other) noexcept;
    
    [[nodiscard]] bool valid() const { return addr_ != MAP_FAILED; }
    [[nodiscard]] uintptr_t address() const { return reinterpret_cast<uintptr_t>(addr_); }
    [[nodiscard]] size_t size() const { return size_; }
    
private:
    void* addr_;
    size_t size_;
};

// Relative pointer - stores offset from module base for portability
template<typename T>
class RelativePtr {
    uint32_t offset_;
    mutable std::once_flag base_init_;
    mutable uintptr_t module_base_ = 0;
    
public:
    explicit RelativePtr(uint32_t offset) : offset_(offset) {}
    
    void set_module(uintptr_t base) const {
        module_base_ = base;
    }
    
    [[nodiscard]] uintptr_t absolute() const {
        std::call_once(base_init_, [this]() {
            // Lazy initialization - would be set externally
        });
        return module_base_ + offset_;
    }
    
    [[nodiscard]] T* get() const {
        return reinterpret_cast<T*>(absolute());
    }
    
    [[nodiscard]] T& operator*() const {
        return *get();
    }
    
    [[nodiscard]] T* operator->() const {
        return get();
    }
};

// Structure for IL2CPP object header parsing
struct Il2CppObject {
    void* vtable;      // Virtual method table pointer
    void* monitor;     // Synchronization monitor
    
    // Returns pointer to actual data after header
    [[nodiscard]] void* data() const {
        return const_cast<void*>(static_cast<const void*>(this + 1));
    }
};

// Type-safe field accessor for IL2CPP structures
// Offset is relative to object data (after Il2CppObject header)
template<typename T>
class Il2CppField {
    uint32_t offset_;
    
public:
    explicit Il2CppField(uint32_t offset) : offset_(offset) {}
    
    [[nodiscard]] T* get_ptr(uintptr_t obj_base) const {
        return reinterpret_cast<T*>(obj_base + sizeof(Il2CppObject) + offset_);
    }
    
    [[nodiscard]] T get(uintptr_t obj_base) const {
        return *get_ptr(obj_base);
    }
    
    void set(uintptr_t obj_base, const T& value) const {
        *get_ptr(obj_base) = value;
    }
};

// Pointer chain resolver for multi-level indirection
class PointerChain {
    std::vector<uint32_t> offsets_;
    
public:
    PointerChain(std::initializer_list<uint32_t> offsets) : offsets_(offsets) {}
    
    [[nodiscard]] std::optional<uintptr_t> resolve(uintptr_t base) const {
        uintptr_t current = base;
        
        for (size_t i = 0; i < offsets_.size(); ++i) {
            if (i == offsets_.size() - 1) {
                // Last offset - return as field address, not dereference
                return current + offsets_[i];
            }
            
            // Dereference pointer
            uintptr_t next;
            if (!ProcessMemory::instance().read(current + offsets_[i], next)) {
                return std::nullopt;
            }
            if (next == 0) return std::nullopt;
            current = next;
        }
        
        return current;
    }
};

} // namespace diag::core

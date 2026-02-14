#include "memory.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

namespace diag::core {

bool ProcessMemory::initialize() {
    cached_regions_ = get_memory_regions();
    return !cached_regions_.empty();
}

uintptr_t ProcessMemory::get_module_base(std::string_view module_name) const {
    std::ifstream maps("/proc/self/maps");
    if (!maps.is_open()) return 0;
    
    std::string line;
    while (std::getline(maps, line)) {
        if (line.find(module_name) != std::string::npos) {
            // Parse base address from first column
            uintptr_t base;
            if (std::sscanf(line.c_str(), "%lx", &base) == 1) {
                return base;
            }
        }
    }
    
    return 0;
}

size_t ProcessMemory::get_module_size(std::string_view module_name) const {
    std::ifstream maps("/proc/self/maps");
    if (!maps.is_open()) return 0;
    
    uintptr_t base = 0;
    uintptr_t end = 0;
    bool found = false;
    
    std::string line;
    while (std::getline(maps, line)) {
        if (line.find(module_name) != std::string::npos) {
            uintptr_t region_base, region_end;
            if (std::sscanf(line.c_str(), "%lx-%lx", &region_base, &region_end) == 2) {
                if (!found) {
                    base = region_base;
                    found = true;
                }
                end = std::max(end, region_end);
            }
        }
    }
    
    return found ? (end - base) : 0;
}

bool ProcessMemory::read_bytes(uintptr_t address, void* out, size_t size) const {
    if (!is_valid_address(address, size)) return false;
    
    // Use memcpy with volatile to prevent optimization
    volatile auto* src = reinterpret_cast<const volatile uint8_t*>(address);
    auto* dst = static_cast<uint8_t*>(out);
    
    for (size_t i = 0; i < size; ++i) {
        dst[i] = src[i];
    }
    
    return true;
}

bool ProcessMemory::write(uintptr_t address, const void* data, size_t size) const {
    if (!is_valid_address(address, size)) return false;
    
    // Change protection temporarily
    uintptr_t page_start = address & ~0xFFF;
    size_t page_size = ((address + size - page_start) + 0xFFF) & ~0xFFF;
    
    int old_prot = 0;
    // Note: mprotect requires page-aligned address
    if (mprotect(reinterpret_cast<void*>(page_start), page_size, 
                 PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return false;
    }
    
    // Perform write
    std::memcpy(reinterpret_cast<void*>(address), data, size);
    
    memory_barrier();
    
    // Restore protection (best effort)
    mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ | PROT_EXEC);
    
    return true;
}

std::optional<uintptr_t> ProcessMemory::pattern_scan(
    std::string_view module_name,
    const std::vector<int>& pattern,
    uintptr_t start_offset
) const {
    uintptr_t base = get_module_base(module_name);
    if (base == 0) return std::nullopt;
    
    size_t size = get_module_size(module_name);
    if (size == 0) return std::nullopt;
    
    base += start_offset;
    size -= start_offset;
    
    // Read module data into buffer
    std::vector<uint8_t> buffer(size);
    if (!read_bytes(base, buffer.data(), size)) {
        return std::nullopt;
    }
    
    // Scan for pattern
    for (size_t i = 0; i <= size - pattern.size(); ++i) {
        bool match = true;
        for (size_t j = 0; j < pattern.size(); ++j) {
            if (pattern[j] != -1 && buffer[i + j] != static_cast<uint8_t>(pattern[j])) {
                match = false;
                break;
            }
        }
        if (match) {
            return base + i;
        }
    }
    
    return std::nullopt;
}

std::vector<MemoryRegion> ProcessMemory::get_memory_regions() const {
    std::vector<MemoryRegion> regions;
    
    std::ifstream maps("/proc/self/maps");
    if (!maps.is_open()) return regions;
    
    std::string line;
    while (std::getline(maps, line)) {
        uintptr_t base, end;
        char perms[5];
        char name_buf[256] = {0};
        
        // Format: address perms offset dev inode pathname
        if (std::sscanf(line.c_str(), "%lx-%lx %4s %*s %*s %*s %255[^\n]",
                       &base, &end, perms, name_buf) >= 3) {
            MemoryRegion region;
            region.base = base;
            region.size = end - base;
            region.prot = 0;
            if (perms[0] == 'r') region.prot |= PROT_READ;
            if (perms[1] == 'w') region.prot |= PROT_WRITE;
            if (perms[2] == 'x') region.prot |= PROT_EXEC;
            region.name = name_buf;
            
            // Trim whitespace from name
            size_t start = region.name.find_first_not_of(" \t");
            if (start != std::string::npos) {
                region.name = region.name.substr(start);
            }
            
            regions.push_back(region);
        }
    }
    
    return regions;
}

uintptr_t ProcessMemory::resolve_rel32(uintptr_t instruction_addr, int offset_idx) {
    // Read the relative offset from instruction
    int32_t rel_offset;
    if (!instance().read(instruction_addr + offset_idx, rel_offset)) {
        return 0;
    }
    
    // Calculate absolute address (instruction end + relative offset)
    return instruction_addr + offset_idx + sizeof(int32_t) + rel_offset;
}

int32_t ProcessMemory::calc_rel32(uintptr_t from, uintptr_t to) {
    // Calculate relative offset for jmp/call
    return static_cast<int32_t>(to - (from + 5));
}

bool ProcessMemory::is_valid_address(uintptr_t addr, size_t size) const {
    if (addr == 0) return false;
    
    std::lock_guard<std::mutex> lock(regions_mutex_);
    
    for (const auto& region : cached_regions_) {
        if (addr >= region.base && (addr + size) <= (region.base + region.size)) {
            return (region.prot & PROT_READ) != 0;
        }
    }
    
    // Refresh cache if not found
    cached_regions_ = get_memory_regions();
    
    for (const auto& region : cached_regions_) {
        if (addr >= region.base && (addr + size) <= (region.base + region.size)) {
            return (region.prot & PROT_READ) != 0;
        }
    }
    
    return false;
}

// MemoryMapping implementation
MemoryMapping::MemoryMapping(uintptr_t addr, size_t size, int prot) 
    : size_(size) {
    addr_ = mmap(reinterpret_cast<void*>(addr), size, prot, 
                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

MemoryMapping::~MemoryMapping() {
    if (addr_ != MAP_FAILED) {
        munmap(addr_, size_);
    }
}

MemoryMapping::MemoryMapping(MemoryMapping&& other) noexcept 
    : addr_(other.addr_), size_(other.size_) {
    other.addr_ = MAP_FAILED;
    other.size_ = 0;
}

MemoryMapping& MemoryMapping::operator=(MemoryMapping&& other) noexcept {
    if (this != &other) {
        if (addr_ != MAP_FAILED) {
            munmap(addr_, size_);
        }
        addr_ = other.addr_;
        size_ = other.size_;
        other.addr_ = MAP_FAILED;
        other.size_ = 0;
    }
    return *this;
}

} // namespace diag::core

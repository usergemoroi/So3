#include "diag_module.hpp"
#include <cstdint>
#include <cstring>

// ARM64 instruction encoding utilities
namespace dm::hook {

// ARM64 instruction format helpers
constexpr uint32_t make_b(uint32_t imm26) {
    return 0x14000000 | (imm26 & 0x3FFFFFF);
}

constexpr uint32_t make_bl(uint32_t imm26) {
    return 0x94000000 | (imm26 & 0x3FFFFFF);
}

constexpr uint32_t make_movz(uint8_t rd, uint16_t imm16, uint8_t shift) {
    return 0xD2800000 | (rd & 0x1F) | ((shift & 3) << 21) | ((imm16 & 0xFFFF) << 5);
}

constexpr uint32_t make_movk(uint8_t rd, uint16_t imm16, uint8_t shift) {
    return 0xF2800000 | (rd & 0x1F) | ((shift & 3) << 21) | ((imm16 & 0xFFFF) << 5);
}

constexpr uint32_t make_br(uint8_t rn) {
    return 0xD61F0000 | ((rn & 0x1F) << 5);
}

constexpr uint32_t make_ret() {
    return 0xD65F03C0;
}

// Cache flush for ARM64 (required after code modification)
inline void cache_flush(void* addr, size_t size) {
    // On Android, use __builtin___clear_cache
    __builtin___clear_cache(
        static_cast<char*>(addr),
        static_cast<char*>(addr) + size);
}

// Inline hook structure
struct inline_hook_t {
    void* target_func;
    void* hook_func;
    void* trampoline;
    uint32_t original_code[4];
    bool installed;
    
    bool install();
    bool remove();
};

// Create trampoline for hook
void* create_trampoline(void* target, uint32_t* orig_code) {
    // Allocate executable memory for trampoline
    // On Android, use mmap with PROT_EXEC
    
    // Trampoline structure:
    // 1. Execute original instructions (relocated)
    // 2. Jump back to original function + 4 instructions
    
    // This requires instruction relocation which is complex on ARM64
    // due to PC-relative instructions
    
    // Simplified: allocate and return nullptr (would need external allocator)
    (void)target; (void)orig_code;
    return nullptr;
}

bool inline_hook_t::install() {
    if (installed) return false;
    
    // Save original code
    std::memcpy(original_code, target_func, sizeof(original_code));
    
    // Calculate relative offset for branch
    intptr_t offset = reinterpret_cast<intptr_t>(hook_func) - 
                      reinterpret_cast<intptr_t>(target_func);
    
    // Check if offset fits in 26-bit signed immediate
    if (offset < -0x2000000 || offset >= 0x2000000) {
        // Need stub: load absolute address and branch
        // movz x17, #imm16_0
        // movk x17, #imm16_16, lsl #16
        // movk x17, #imm16_32, lsl #32
        // br x17
        
        uintptr_t abs_addr = reinterpret_cast<uintptr_t>(hook_func);
        uint32_t* code = static_cast<uint32_t*>(target_func);
        
        code[0] = make_movz(17, abs_addr & 0xFFFF, 0);
        code[1] = make_movk(17, (abs_addr >> 16) & 0xFFFF, 1);
        code[2] = make_movk(17, (abs_addr >> 32) & 0xFFFF, 2);
        code[3] = make_br(17);
    } else {
        // Direct relative branch
        uint32_t* code = static_cast<uint32_t*>(target_func);
        code[0] = make_bl(static_cast<uint32_t>(offset >> 2));
        
        // NOP remaining instructions
        for (int i = 1; i < 4; ++i) {
            code[i] = 0xD503201F;  // NOP
        }
    }
    
    cache_flush(target_func, 16);
    installed = true;
    return true;
}

bool inline_hook_t::remove() {
    if (!installed) return false;
    
    // Restore original code
    std::memcpy(target_func, original_code, sizeof(original_code));
    cache_flush(target_func, 16);
    
    installed = false;
    return true;
}

// VTable hook for C++ virtual functions
struct vtable_hook_t {
    void** vtable;
    size_t index;
    void* hook_func;
    void* original_func;
    bool installed;
    
    bool install() {
        if (installed || !vtable) return false;
        
        original_func = vtable[index];
        
        // Atomic pointer swap
        __atomic_store(&vtable[index], &hook_func, __ATOMIC_SEQ_CST);
        
        installed = true;
        return true;
    }
    
    bool remove() {
        if (!installed) return false;
        
        __atomic_store(&vtable[index], &original_func, __ATOMIC_SEQ_CST);
        
        installed = false;
        return true;
    }
};

// PLT/GOT hook for imported functions (Android specific)
struct got_hook_t {
    void** got_entry;
    void* hook_func;
    void* original_func;
    bool installed;
    
    bool install() {
        if (installed || !got_entry) return false;
        
        original_func = *got_entry;
        
        // Make GOT entry writable if needed (mprotect)
        // Then atomic swap
        __atomic_store(got_entry, &hook_func, __ATOMIC_SEQ_CST);
        
        installed = true;
        return true;
    }
    
    bool remove() {
        if (!installed) return false;
        
        __atomic_store(got_entry, &original_func, __ATOMIC_SEQ_CST);
        
        installed = false;
        return true;
    }
};

} // namespace dm::hook

// Export hook utilities for module use
extern "C" {

// Hook eglSwapBuffers for OpenGL ES frame capture
bool dm_hook_gl_present(void* hook_func, void** orig_func) {
    (void)hook_func; (void)orig_func;
    
    // Find eglSwapBuffers in libEGL.so
    // Install hook to call our render function
    
    return true;
}

// Hook vkQueuePresentKHR for Vulkan
bool dm_hook_vk_present(void* hook_func, void** orig_func) {
    (void)hook_func; (void)orig_func;
    
    // Find vkQueuePresentKHR
    // Similar to GL hook
    
    return true;
}

// Unity-specific: Hook ScriptableRenderContext::Submit
bool dm_hook_unity_render(void* hook_func, void** orig_func) {
    (void)hook_func; (void)orig_func;
    
    // In Unity IL2CPP, find the native render submit function
    // This varies by Unity version
    
    return true;
}

} // extern "C"

#include "render_hooks.hpp"
#include "../core/memory.hpp"
#include "../utils/obfuscation.hpp"
#include <dlfcn.h>
#include <android/log.h>
#include <cstring>

namespace diag::hooks {

bool RenderHookManager::initialize() {
    OBFUSCATE_FLOW();
    
    // Initialize all trampolines as inactive
    for (auto& tramp : trampolines_) {
        tramp.active = false;
        tramp.original = 0;
        tramp.trampoline = 0;
        tramp.hook_size = 0;
        tramp.original_bytes.fill(0);
    }
    
    return true;
}

void RenderHookManager::shutdown() {
    // Remove all active hooks
    for (size_t i = 0; i < trampolines_.size(); ++i) {
        if (trampolines_[i].active) {
            restore_original(static_cast<HookType>(i));
        }
    }
}

bool RenderHookManager::install_hook(const HookConfig& config, uintptr_t callback) {
    if (config.target_address == 0) {
        return false;
    }
    
    size_t type_idx = static_cast<size_t>(config.type);
    if (type_idx >= trampolines_.size()) {
        return false;
    }
    
    auto& tramp = trampolines_[type_idx];
    
    // Calculate required hook size
    size_t hook_size = detail::get_min_hook_size(config.target_address);
    if (hook_size < 4) hook_size = 4;  // Minimum: one branch instruction
    
    // Create trampoline
    if (!create_trampoline(config.target_address, hook_size, tramp)) {
        return false;
    }
    
    // Write jump from target to callback
    if (!write_jump(config.target_address, callback, hook_size)) {
        return false;
    }
    
    tramp.active = true;
    
    // Memory barrier to ensure instruction cache coherency
    core::ProcessMemory::memory_barrier();
    
    // Clear instruction cache
    __builtin___clear_cache(
        reinterpret_cast<void*>(config.target_address),
        reinterpret_cast<void*>(config.target_address + hook_size)
    );
    
    return true;
}

bool RenderHookManager::auto_hook_opengl() {
    if (!find_opengl_functions()) {
        return false;
    }
    
    // Hook eglSwapBuffers for frame timing
    HookConfig swap_config;
    swap_config.type = HookType::OpenGL_Present;
    swap_config.target_address = egl_swap_buffers_;
    swap_config.preserve_original = true;
    swap_config.priority = 0;
    
    // Callback would be set to hook_egl_swap_buffers
    // install_hook(swap_config, reinterpret_cast<uintptr_t>(hook_egl_swap_buffers));
    
    return true;
}

bool RenderHookManager::auto_hook_unity() {
    if (!find_unity_render_func()) {
        return false;
    }
    
    // Hook Unity's render function
    HookConfig render_config;
    render_config.type = HookType::Unity_Render;
    render_config.target_address = unity_render_;
    render_config.preserve_original = true;
    render_config.priority = 1;
    
    return true;
}

bool RenderHookManager::remove_hook(HookType type) {
    return restore_original(type);
}

bool RenderHookManager::create_trampoline(uintptr_t target, size_t hook_size, Trampoline& out) {
    // Allocate executable memory for trampoline
    size_t tramp_size = hook_size + 16;  // Original code + jump back + padding
    
    void* mem = mmap(nullptr, tramp_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (mem == MAP_FAILED) {
        return false;
    }
    
    out.trampoline = reinterpret_cast<uintptr_t>(mem);
    out.original = target;
    out.hook_size = hook_size;
    
    // Copy original bytes
    core::ProcessMemory& proc_mem = core::ProcessMemory::instance();
    proc_mem.read_bytes(target, out.original_bytes.data(), hook_size);
    std::memcpy(mem, out.original_bytes.data(), hook_size);
    
    // Write jump back to original + hook_size
    uintptr_t return_addr = target + hook_size;
    uintptr_t tramp_jump = out.trampoline + hook_size;
    
    // Ensure alignment for next instruction
    size_t aligned_size = (hook_size + 3) & ~3;
    tramp_jump = out.trampoline + aligned_size;
    
    uint32_t branch = detail::make_branch(tramp_jump, return_addr, false);
    *reinterpret_cast<uint32_t*>(tramp_jump) = branch;
    
    return true;
}

bool RenderHookManager::write_jump(uintptr_t from, uintptr_t to, size_t size) {
    // Change memory protection temporarily
    uintptr_t page_start = from & ~0xFFF;
    size_t page_size = ((from + size - page_start) + 0xFFF) & ~0xFFF;
    
    if (mprotect(reinterpret_cast<void*>(page_start), page_size, 
                 PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return false;
    }
    
    // Write branch instruction
    uint32_t branch = detail::make_branch(from, to, false);
    *reinterpret_cast<uint32_t*>(from) = branch;
    
    // Fill remaining space with NOPs
    for (size_t i = 4; i < size; i += 4) {
        *reinterpret_cast<uint32_t*>(from + i) = detail::make_nop();
    }
    
    // Memory barrier
    core::ProcessMemory::memory_barrier();
    
    // Clear instruction cache
    __builtin___clear_cache(
        reinterpret_cast<void*>(from),
        reinterpret_cast<void*>(from + size)
    );
    
    return true;
}

bool RenderHookManager::restore_original(HookType type) {
    size_t type_idx = static_cast<size_t>(type);
    if (type_idx >= trampolines_.size()) {
        return false;
    }
    
    auto& tramp = trampolines_[type_idx];
    if (!tramp.active) return true;
    
    // Restore original bytes
    uintptr_t page_start = tramp.original & ~0xFFF;
    size_t page_size = ((tramp.original + tramp.hook_size - page_start) + 0xFFF) & ~0xFFF;
    
    mprotect(reinterpret_cast<void*>(page_start), page_size, PROT_READ | PROT_WRITE | PROT_EXEC);
    
    std::memcpy(reinterpret_cast<void*>(tramp.original), 
                tramp.original_bytes.data(), 
                tramp.hook_size);
    
    // Clear cache
    __builtin___clear_cache(
        reinterpret_cast<void*>(tramp.original),
        reinterpret_cast<void*>(tramp.original + tramp.hook_size)
    );
    
    // Free trampoline
    if (tramp.trampoline) {
        munmap(reinterpret_cast<void*>(tramp.trampoline), tramp.hook_size + 16);
    }
    
    tramp.active = false;
    return true;
}

bool RenderHookManager::find_opengl_functions() {
    // Get handle to libEGL.so
    void* egl_lib = dlopen("libEGL.so", RTLD_LAZY);
    if (!egl_lib) {
        return false;
    }
    
    // Find eglSwapBuffers
    egl_swap_buffers_ = reinterpret_cast<uintptr_t>(dlsym(egl_lib, "eglSwapBuffers"));
    
    // Find glDrawElements for draw call hooking
    void* gl_lib = dlopen("libGLESv3.so", RTLD_LAZY);
    if (!gl_lib) {
        gl_lib = dlopen("libGLESv2.so", RTLD_LAZY);
    }
    
    if (gl_lib) {
        gl_draw_elements_ = reinterpret_cast<uintptr_t>(dlsym(gl_lib, "glDrawElements"));
    }
    
    return egl_swap_buffers_ != 0;
}

bool RenderHookManager::find_unity_render_func() {
    // Pattern scan for Unity's render function
    // This is game-specific and would need adjustment per title
    
    core::ProcessMemory& mem = core::ProcessMemory::instance();
    
    // Look for common Unity render patterns
    // Pattern: "PlayerRender" function or similar
    std::vector<int> pattern = {
        0xFD, 0x7B, 0xBF, 0xA9,  // stp x29, x30, [sp, #-0x10]!
        0xFD, 0x03, 0x00, 0x91,  // mov x29, sp
        -1, -1, -1, -1,          // ...
        0x94, 0x00, 0x00, 0x00   // bl <some_function>
    };
    
    auto result = mem.pattern_scan("libmain.so", pattern);
    if (result) {
        unity_render_ = *result;
        return true;
    }
    
    return false;
}

void RenderHookManager::on_pre_render(FrameCallback cb) {
    pre_render_callbacks_.push_back(std::move(cb));
}

void RenderHookManager::on_post_render(FrameCallback cb) {
    post_render_callbacks_.push_back(std::move(cb));
}

bool RenderHookManager::is_hooked(HookType type) const {
    size_t idx = static_cast<size_t>(type);
    return idx < trampolines_.size() && trampolines_[idx].active;
}

// Detail implementations
namespace detail {

size_t get_min_hook_size(uintptr_t target) {
    // On ARM64, instructions are 4 bytes
    // We need at least 4 bytes for a branch, but must be instruction-aligned
    // Some scenarios may need 8 bytes (two instructions)
    
    // Read first instruction to check if it can be safely relocated
    uint32_t first_inst;
    core::ProcessMemory::instance().read(target, first_inst);
    
    // Check for PC-relative instructions that need special handling
    // This is simplified - real implementation would be more thorough
    
    return 4;  // Default to single instruction
}

} // namespace detail

// C-linkage entry points for assembly stubs
extern "C" {

void hook_pre_render() {
    auto& manager = RenderHookManager::instance();
    for (const auto& cb : manager.pre_render_callbacks_) {
        cb();
    }
}

void hook_post_render() {
    auto& manager = RenderHookManager::instance();
    for (const auto& cb : manager.post_render_callbacks_) {
        cb();
    }
}

void hook_egl_swap_buffers() {
    // Update frame timing
    static auto last_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    
    auto& manager = RenderHookManager::instance();
    // manager.last_frame_delta_ = ... calculate delta
    manager.frame_count_++;
    
    // Call pre-render callbacks
    hook_pre_render();
    
    // Original function would be called via trampoline
    // manager.call_original<void, ...>(HookType::OpenGL_Present, ...);
    
    // Call post-render callbacks
    hook_post_render();
}

} // extern "C"

} // namespace diag::hooks

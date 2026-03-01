#pragma once

#include <cstdint>
#include <functional>
#include <array>

namespace diag::hooks {

// Hook types for different render APIs
enum class HookType {
    OpenGL_Present,         // eglSwapBuffers
    OpenGL_Draw,            // glDrawElements/Arrays
    Vulkan_QueueSubmit,     // vkQueueSubmit
    Unity_Render,           // Unity rendering event
    Custom                  // User-defined hook point
};

// Hook configuration
struct HookConfig {
    HookType type;
    uintptr_t target_address;  // Function to hook (0 for auto-detect)
    bool preserve_original;
    int priority;              // Hook execution order
};

// Original function preservation
struct Trampoline {
    uintptr_t original;
    uintptr_t trampoline;
    size_t hook_size;
    std::array<uint8_t, 32> original_bytes;
    bool active;
};

// Hook manager for render pipeline interception
class RenderHookManager {
public:
    static RenderHookManager& instance() {
        static RenderHookManager inst;
        return inst;
    }
    
    bool initialize();
    void shutdown();
    
    // Install hook at specific address
    bool install_hook(const HookConfig& config, uintptr_t callback);
    
    // Auto-detect and hook rendering functions
    bool auto_hook_opengl();
    bool auto_hook_vulkan();
    bool auto_hook_unity();
    
    // Remove specific hook
    bool remove_hook(HookType type);
    
    // Call original function through trampoline
    template<typename Ret, typename... Args>
    Ret call_original(HookType type, Args... args) {
        auto& tramp = trampolines_[static_cast<size_t>(type)];
        if (!tramp.active) return Ret{};
        
        using FuncType = Ret(*)(Args...);
        auto* func = reinterpret_cast<FuncType>(tramp.trampoline);
        return func(args...);
    }
    
    // Frame callback registration
    using FrameCallback = std::function<void()>;
    void on_pre_render(FrameCallback cb);
    void on_post_render(FrameCallback cb);
    
    // Get frame timing
    [[nodiscard]] float get_frame_delta() const { return last_frame_delta_; }
    [[nodiscard]] int get_frame_count() const { return frame_count_; }
    
    // Check if hooks are active
    [[nodiscard]] bool is_hooked(HookType type) const;

private:
    RenderHookManager() = default;
    
    bool create_trampoline(uintptr_t target, size_t hook_size, Trampoline& out);
    bool write_jump(uintptr_t from, uintptr_t to, size_t size);
    bool restore_original(HookType type);
    
    bool find_opengl_functions();
    bool find_unity_render_func();
    
    std::array<Trampoline, 16> trampolines_;
    std::vector<FrameCallback> pre_render_callbacks_;
    std::vector<FrameCallback> post_render_callbacks_;
    
    float last_frame_delta_ = 0.0f;
    int frame_count_ = 0;
    void* timer_context_ = nullptr;
    
    // Cached function addresses
    uintptr_t egl_swap_buffers_ = 0;
    uintptr_t gl_draw_elements_ = 0;
    uintptr_t unity_render_ = 0;
};

// Inline hook implementation for ARM64
namespace detail {
    // Generate ARM64 branch instruction
    [[nodiscard]] inline uint32_t make_branch(uintptr_t from, uintptr_t to, bool link = false) {
        int64_t offset = static_cast<int64_t>(to) - static_cast<int64_t>(from);
        uint32_t imm26 = (offset >> 2) & 0x3FFFFFF;
        uint32_t opcode = link ? 0x94000000 : 0x14000000;
        return opcode | imm26;
    }
    
    // Generate NOP
    [[nodiscard]] constexpr uint32_t make_nop() {
        return 0xD503201F;  // NOP instruction
    }
    
    // Calculate minimum hook size (must cover whole instructions)
    [[nodiscard]] size_t get_min_hook_size(uintptr_t target);
}

// Hook entry points (called from assembly stubs)
extern "C" {
    void hook_pre_render();
    void hook_post_render();
    void hook_egl_swap_buffers();
}

} // namespace diag::hooks

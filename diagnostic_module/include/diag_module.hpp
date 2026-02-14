#pragma once

#include "secure_types.hpp"
#include "math_ops.hpp"
#include "memory_layout.hpp"
#include "renderer.hpp"
#include "aim_system.hpp"

#include <cstdint>
#include <thread>
#include <atomic>
#include <mutex>

// Forward declarations for render hook
struct ImGuiContext;
struct ImFont;

namespace dm {

// Module initialization modes
enum class init_mode : uint32_t {
    MANUAL_THREAD,      // Create dedicated thread
    RENDER_HOOK,        // Hook existing render function
    UNITY_CALLBACK,     // Use Unity's native plugin interface
    LATE_ATTACH         // Attach to already running game
};

// Runtime configuration
struct module_config_t {
    init_mode mode = init_mode::RENDER_HOOK;
    
    // Feature toggles
    bool enable_overlay = true;
    bool enable_esp = true;
    bool enable_skeleton = true;
    bool enable_aim = false;  // Disabled by default for safety
    
    // Render settings
    float esp_max_distance = 300.0f;
    bool esp_fade_distance = true;
    bool esp_health_color = true;
    
    // Logging (disabled in release for stealth)
    bool enable_debug_log = false;
    
    // Anti-detection
    bool obfuscate_strings = true;
    bool insert_junk_code = true;
    bool randomize_timing = true;
};

// Main diagnostic module class
class diagnostic_module {
public:
    diagnostic_module();
    ~diagnostic_module();
    
    // Non-copyable, non-movable (singleton-like)
    diagnostic_module(const diagnostic_module&) = delete;
    diagnostic_module& operator=(const diagnostic_module&) = delete;
    
    // Get singleton instance
    [[nodiscard]] static diagnostic_module& instance();
    
    // Initialize with module base address (from dlopen or memory scan)
    bool initialize(uintptr_t game_module_base, size_t module_size);
    
    // Shutdown and cleanup
    void shutdown();
    
    // Main render callback (called from hooked render function)
    void on_render(ImDrawList* draw_list, int screen_w, int screen_h);
    
    // Update tick (called from game thread or dedicated thread)
    void update();
    
    // Configuration access
    module_config_t& config() { return config_; }
    const module_config_t& config() const { return config_; }
    
    // Check if initialized
    [[nodiscard]] bool is_initialized() const { return initialized_.load(); }
    
    // Safe cleanup on library unload
    static void cleanup();
    
private:
    bool setup_imgui();
    bool setup_hooks();
    void remove_hooks();
    
    void update_entities();
    void render_esp();
    void render_overlay();
    void process_aim();
    
    // Stealth logging (only when debug enabled)
    void log_internal(const char* msg);
    
    std::atomic<bool> initialized_{false};
    std::atomic<bool> should_exit_{false};
    
    module_config_t config_;
    render_context_t render_ctx_;
    aim::aim_controller aim_controller_;
    aim::aim_state_t aim_state_;
    
    // Entity cache (double buffered for thread safety)
    static constexpr size_t MAX_ENTITIES = 64;
    entity_cache_t entity_cache_[MAX_ENTITIES];
    std::atomic<uint32_t> entity_count_{0};
    
    // ImGui state
    ImGuiContext* imgui_ctx_ = nullptr;
    ImFont* default_font_ = nullptr;
    
    // Threading
    std::thread update_thread_;
    std::mutex render_mutex_;
    
    // Module info
    uintptr_t module_base_ = 0;
    size_t module_size_ = 0;
    
    // Original render function (for unhooking)
    void* original_present_ = nullptr;
    void* original_draw_indexed_ = nullptr;
};

// C API for external loader integration
extern "C" {
    // Entry point for library load
    [[gnu::visibility("default")]]
    bool dm_initialize(uintptr_t game_base, size_t game_size);
    
    // Cleanup entry point
    [[gnu::visibility("default")]]
    void dm_shutdown();
    
    // Render callback (to be called from hooked function)
    [[gnu::visibility("default")]]
    void dm_render(void* draw_list, int w, int h);
}

} // namespace dm

// Convenience macro for module access
#define DM_MODULE dm::diagnostic_module::instance()

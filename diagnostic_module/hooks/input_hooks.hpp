#pragma once

#include <cstdint>
#include <functional>

namespace diag::hooks {

// Input event types
enum class InputEventType {
    TouchDown,
    TouchMove,
    TouchUp,
    KeyDown,
    KeyUp,
    Motion
};

struct TouchEvent {
    int pointer_id;
    float x, y;
    float pressure;
    int64_t timestamp;
};

// Input callback type
using InputCallback = std::function<bool(InputEventType type, const TouchEvent& event)>;

// Input hook manager for touch/motion event interception
class InputHookManager {
public:
    static InputHookManager& instance() {
        static InputHookManager inst;
        return inst;
    }
    
    bool initialize();
    void shutdown();
    
    // Hook Android input system
    bool hook_input_queue();
    bool hook_native_activity();
    
    // Register input callbacks
    void register_callback(InputCallback cb, int priority = 0);
    
    // Process input event (called from hooked function)
    bool process_input(InputEventType type, const TouchEvent& event);
    
    // Check if touch is in overlay area
    [[nodiscard]] bool is_overlay_touch(float x, float y) const;
    
    // Set overlay area (for input passthrough)
    void set_overlay_region(float x, float y, float width, float height);
    
    // Enable/disable input interception
    void set_intercept_enabled(bool enabled) { intercept_enabled_ = enabled; }
    [[nodiscard]] bool is_intercept_enabled() const { return intercept_enabled_; }

private:
    InputHookManager() = default;
    
    struct InputHandler {
        InputCallback callback;
        int priority;
    };
    
    std::vector<InputHandler> callbacks_;
    
    // Overlay region for UI interaction
    float overlay_x_ = 0.0f;
    float overlay_y_ = 0.0f;
    float overlay_width_ = 0.0f;
    float overlay_height_ = 0.0f;
    
    bool intercept_enabled_ = true;
    
    // Original function pointers
    void* original_dispatch_ = nullptr;
};

} // namespace diag::hooks

#pragma once

#include "../math/vector.hpp"
#include "../features/scene_visualizer.hpp"
#include <cstdint>
#include <functional>

// Forward declarations for ImGui
struct ImDrawData;
struct ImFont;

namespace diag::renderer {

// Graphics API type
enum class GraphicsAPI {
    Unknown,
    OpenGLES2,
    OpenGLES3,
    Vulkan
};

// Overlay configuration
struct OverlayConfig {
    // Position and size
    int screen_width = 1920;
    int screen_height = 1080;
    float ui_scale = 1.0f;
    
    // Style
    float window_rounding = 4.0f;
    float frame_rounding = 2.0f;
    float alpha = 0.95f;
    
    // Colors (ImGui style)
    float col_text[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float col_window_bg[4] = {0.06f, 0.06f, 0.06f, 0.94f};
    float col_border[4] = {0.43f, 0.43f, 0.50f, 0.50f};
    float col_button[4] = {0.26f, 0.59f, 0.98f, 0.40f};
    float col_button_hover[4] = {0.26f, 0.59f, 0.98f, 1.00f};
    
    // Features
    bool show_menu = true;
    bool show_stats = true;
    bool show_entity_list = false;
};

// Statistics display data
struct FrameStats {
    float fps;
    float frame_time_ms;
    int entity_count;
    int draw_calls;
    float memory_usage_mb;
};

// Main overlay class using Dear ImGui
class ImGuiOverlay {
public:
    static ImGuiOverlay& instance() {
        static ImGuiOverlay inst;
        return inst;
    }
    
    // Initialize with existing graphics context
    bool initialize(void* native_window, void* gl_context, GraphicsAPI api);
    
    // Shutdown and cleanup
    void shutdown();
    
    // Begin frame (call at start of render loop)
    void begin_frame();
    
    // End frame and render (call at end of render loop)
    void end_frame();
    
    // Render main menu
    void render_menu();
    
    // Render statistics overlay
    void render_stats(const FrameStats& stats);
    
    // Render entity debug list
    void render_entity_list(const std::vector<core::EntityInfo>& entities);
    
    // Render visualization primitives
    void render_visualization(const features::SceneVisualizer& visualizer);
    
    // Input handling
    void process_touch_input(float x, float y, bool down);
    
    // Configuration
    void set_config(const OverlayConfig& config) { config_ = config; }
    [[nodiscard]] const OverlayConfig& get_config() const { return config_; }
    
    // Visibility toggle
    void toggle_menu() { config_.show_menu = !config_.show_menu; }
    void set_visible(bool visible) { visible_ = visible; }
    [[nodiscard]] bool is_visible() const { return visible_; }
    
    // Graphics API detection
    [[nodiscard]] static GraphicsAPI detect_api();
    
    // Check if initialized
    [[nodiscard]] bool is_initialized() const { return initialized_; }
    
    // Get draw data for custom rendering
    [[nodiscard]] ImDrawData* get_draw_data();

private:
    ImGuiOverlay() = default;
    
    bool setup_imgui(void* native_window, void* gl_context);
    bool setup_fonts();
    void apply_style();
    
    bool initialized_ = false;
    bool visible_ = true;
    GraphicsAPI graphics_api_ = GraphicsAPI::Unknown;
    OverlayConfig config_;
    
    // ImGui context pointer (opaque)
    void* imgui_context_ = nullptr;
    
    // Font handles
    ImFont* default_font_ = nullptr;
    ImFont* icon_font_ = nullptr;
};

// OpenGL ES render backend
class OpenGLESRenderer {
public:
    bool initialize();
    void shutdown();
    
    void new_frame(int width, int height);
    void render_draw_data(ImDrawData* draw_data);
    
    // Draw visualization primitives
    void draw_line(const math::Vec2& start, const math::Vec2& end, 
                   uint32_t color, float thickness);
    void draw_rect(const math::Vec2& min, const math::Vec2& max, 
                   uint32_t color, float thickness);
    void draw_filled_rect(const math::Vec2& min, const math::Vec2& max, 
                          uint32_t fill_color, uint32_t border_color, float thickness);
    void draw_circle(const math::Vec2& center, float radius, 
                     uint32_t color, float thickness, int segments);
    void draw_text(const math::Vec2& pos, const char* text, 
                   uint32_t color, float scale);

private:
    uint32_t shader_program_ = 0;
    uint32_t vbo_ = 0;
    uint32_t vao_ = 0;
    
    int attrib_position_ = 0;
    int attrib_uv_ = 0;
    int attrib_color_ = 0;
    int uniform_proj_ = 0;
};

// Vulkan render backend (stub)
class VulkanRenderer {
public:
    bool initialize(void* device, void* queue);
    void shutdown();
    void render(void* command_buffer);
};

} // namespace diag::renderer

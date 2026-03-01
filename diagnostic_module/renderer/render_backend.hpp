#pragma once

#include "../math/vector.hpp"
#include <vector>
#include <cstdint>

namespace diag::renderer {

// Vertex structure for batch rendering
struct Vertex {
    float pos[2];      // Position
    float uv[2];       // Texture coordinates
    uint32_t col;      // Color (RGBA)
};

// Extended OpenGL ES renderer with batching
class OpenGLESRenderer {
public:
    bool initialize();
    void shutdown();
    
    void new_frame(int width, int height);
    void render_draw_data(void* draw_data);
    
    // Individual primitive drawing
    void draw_line(const math::Vec2& start, const math::Vec2& end, 
                   uint32_t color, float thickness);
    void draw_rect(const math::Vec2& min, const math::Vec2& max, 
                   uint32_t color, float thickness);
    void draw_filled_rect(const math::Vec2& min, const math::Vec2& max, 
                          uint32_t fill_color, uint32_t border_color, float thickness);
    void draw_circle(const math::Vec2& center, float radius, 
                     uint32_t color, float thickness, int segments = 16);
    void draw_text(const math::Vec2& pos, const char* text, 
                   uint32_t color, float scale);
    
    // Batch rendering for efficiency
    void begin_batch();
    void add_line_to_batch(const math::Vec2& start, const math::Vec2& end, uint32_t color);
    void end_batch();

private:
    uint32_t shader_program_ = 0;
    uint32_t vbo_ = 0;
    uint32_t vao_ = 0;
    
    int attrib_position_ = 0;
    int attrib_uv_ = 0;
    int attrib_color_ = 0;
    int uniform_proj_ = 0;
    
    std::vector<Vertex> batch_vertices_;
};

// Vulkan render backend stub
class VulkanRenderer {
public:
    bool initialize(void* device, void* queue);
    void shutdown();
    void render(void* command_buffer);
    
    void draw_line(const math::Vec2& start, const math::Vec2& end, uint32_t color, float thickness);
    void draw_rect(const math::Vec2& min, const math::Vec2& max, uint32_t color, float thickness);
    void draw_filled_rect(const math::Vec2& min, const math::Vec2& max, 
                          uint32_t fill_color, uint32_t border_color, float thickness);
};

} // namespace diag::renderer

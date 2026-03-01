#include "render_backend.hpp"
#include <GLES3/gl3.h>
#include <cmath>

namespace diag::renderer {

// Minimal vertex shader for 2D overlay rendering
constexpr const char* vertex_shader_src = R"(#version 300 es
    precision mediump float;
    layout(location = 0) in vec2 a_position;
    layout(location = 1) in vec2 a_texcoord;
    layout(location = 2) in vec4 a_color;
    
    uniform mat4 u_projection;
    
    out vec2 v_texcoord;
    out vec4 v_color;
    
    void main() {
        gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
        v_texcoord = a_texcoord;
        v_color = a_color;
    }
)";

// Fragment shader for textured/colored output
constexpr const char* fragment_shader_src = R"(#version 300 es
    precision mediump float;
    in vec2 v_texcoord;
    in vec4 v_color;
    
    uniform sampler2D u_texture;
    uniform bool u_use_texture;
    
    out vec4 frag_color;
    
    void main() {
        if (u_use_texture) {
            frag_color = v_color * texture(u_texture, v_texcoord);
        } else {
            frag_color = v_color;
        }
    }
)";

// Simple color-only fragment shader
constexpr const char* color_fragment_src = R"(#version 300 es
    precision mediump float;
    in vec2 v_texcoord;
    in vec4 v_color;
    out vec4 frag_color;
    
    void main() {
        frag_color = v_color;
    }
)";

static uint32_t compile_shader(uint32_t type, const char* source) {
    uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool OpenGLESRenderer::initialize() {
    // Compile shaders
    uint32_t vertex = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
    uint32_t fragment = compile_shader(GL_FRAGMENT_SHADER, color_fragment_src);
    
    if (!vertex || !fragment) {
        return false;
    }
    
    // Link program
    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vertex);
    glAttachShader(shader_program_, fragment);
    glLinkProgram(shader_program_);
    
    int success;
    glGetProgramiv(shader_program_, GL_LINK_STATUS, &success);
    if (!success) {
        glDeleteProgram(shader_program_);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return false;
    }
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    // Get attribute/uniform locations
    attrib_position_ = 0;  // layout(location = 0)
    attrib_uv_ = 1;        // layout(location = 1)
    attrib_color_ = 2;     // layout(location = 2)
    uniform_proj_ = glGetUniformLocation(shader_program_, "u_projection");
    
    // Create VBO/VAO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    
    // Position attribute
    glEnableVertexAttribArray(attrib_position_);
    glVertexAttribPointer(attrib_position_, 2, GL_FLOAT, GL_FALSE, 
                          sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, pos)));
    
    // UV attribute
    glEnableVertexAttribArray(attrib_uv_);
    glVertexAttribPointer(attrib_uv_, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));
    
    // Color attribute
    glEnableVertexAttribArray(attrib_color_);
    glVertexAttribPointer(attrib_color_, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, col)));
    
    glBindVertexArray(0);
    
    return true;
}

void OpenGLESRenderer::shutdown() {
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (shader_program_) glDeleteProgram(shader_program_);
}

void OpenGLESRenderer::new_frame(int width, int height) {
    // Set up orthographic projection
    // [0, width] x [height, 0] (flip Y for screen coords)
    float projection[4][4] = {
        {2.0f / width, 0.0f, 0.0f, 0.0f},
        {0.0f, -2.0f / height, 0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f, 0.0f},
        {-1.0f, 1.0f, 0.0f, 1.0f}
    };
    
    glUseProgram(shader_program_);
    glUniformMatrix4fv(uniform_proj_, 1, GL_FALSE, &projection[0][0]);
    
    // Enable blending
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable depth test for overlay
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void OpenGLESRenderer::render_draw_data(void* draw_data) {
    (void)draw_data;
    // Would iterate ImDrawData and render
}

void OpenGLESRenderer::draw_line(const math::Vec2& start, const math::Vec2& end,
                                  uint32_t color, float thickness) {
    // Convert color from ABGR to RGBA
    uint8_t a = (color >> 24) & 0xFF;
    uint8_t b = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t r = color & 0xFF;
    uint32_t rgba = (r << 0) | (g << 8) | (b << 16) | (a << 24);
    
    Vertex vertices[2] = {
        {{start.x, start.y}, {0, 0}, rgba},
        {{end.x, end.y}, {0, 0}, rgba}
    };
    
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    
    glLineWidth(thickness);
    glDrawArrays(GL_LINES, 0, 2);
}

void OpenGLESRenderer::draw_rect(const math::Vec2& min, const math::Vec2& max,
                                  uint32_t color, float thickness) {
    uint8_t a = (color >> 24) & 0xFF;
    uint8_t b = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t r = color & 0xFF;
    uint32_t rgba = (r << 0) | (g << 8) | (b << 16) | (a << 24);
    
    // Line loop for rectangle
    Vertex vertices[5] = {
        {{min.x, min.y}, {0, 0}, rgba},
        {{max.x, min.y}, {0, 0}, rgba},
        {{max.x, max.y}, {0, 0}, rgba},
        {{min.x, max.y}, {0, 0}, rgba},
        {{min.x, min.y}, {0, 0}, rgba}  // Close loop
    };
    
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    
    glLineWidth(thickness);
    glDrawArrays(GL_LINE_STRIP, 0, 5);
}

void OpenGLESRenderer::draw_filled_rect(const math::Vec2& min, const math::Vec2& max,
                                         uint32_t fill_color, uint32_t border_color, 
                                         float thickness) {
    // First draw fill as two triangles
    uint8_t fa = (fill_color >> 24) & 0xFF;
    uint8_t fb = (fill_color >> 16) & 0xFF;
    uint8_t fg = (fill_color >> 8) & 0xFF;
    uint8_t fr = fill_color & 0xFF;
    uint32_t fill_rgba = (fr << 0) | (fg << 8) | (fb << 16) | (fa << 24);
    
    Vertex fill_vertices[4] = {
        {{min.x, min.y}, {0, 0}, fill_rgba},
        {{max.x, min.y}, {0, 0}, fill_rgba},
        {{max.x, max.y}, {0, 0}, fill_rgba},
        {{min.x, max.y}, {0, 0}, fill_rgba}
    };
    
    uint16_t indices[6] = {0, 1, 2, 0, 2, 3};
    
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fill_vertices), fill_vertices, GL_STREAM_DRAW);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    
    // Then draw border if requested
    if (thickness > 0.0f) {
        draw_rect(min, max, border_color, thickness);
    }
}

void OpenGLESRenderer::draw_circle(const math::Vec2& center, float radius,
                                    uint32_t color, float thickness, int segments) {
    if (segments < 3) segments = 3;
    
    uint8_t a = (color >> 24) & 0xFF;
    uint8_t b = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t r = color & 0xFF;
    uint32_t rgba = (r << 0) | (g << 8) | (b << 16) | (a << 24);
    
    // Generate circle vertices
    Vertex* vertices = new Vertex[segments + 1];
    
    for (int i = 0; i <= segments; ++i) {
        float theta = 2.0f * 3.14159265f * static_cast<float>(i) / segments;
        vertices[i].pos[0] = center.x + radius * std::cos(theta);
        vertices[i].pos[1] = center.y + radius * std::sin(theta);
        vertices[i].uv[0] = 0;
        vertices[i].uv[1] = 0;
        vertices[i].col = rgba;
    }
    
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, (segments + 1) * sizeof(Vertex), vertices, GL_STREAM_DRAW);
    
    glLineWidth(thickness);
    glDrawArrays(GL_LINE_STRIP, 0, segments + 1);
    
    delete[] vertices;
}

void OpenGLESRenderer::draw_text(const math::Vec2& pos, const char* text,
                                  uint32_t color, float scale) {
    // Simplified text rendering using pre-rasterized font atlas
    // Real implementation would use ImGui's font system
    (void)pos;
    (void)text;
    (void)color;
    (void)scale;
}

// Batch renderer for efficient primitive rendering
void OpenGLESRenderer::begin_batch() {
    batch_vertices_.clear();
}

void OpenGLESRenderer::end_batch() {
    if (batch_vertices_.empty()) return;
    
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, 
                 batch_vertices_.size() * sizeof(Vertex),
                 batch_vertices_.data(), 
                 GL_STREAM_DRAW);
    
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(batch_vertices_.size()));
}

void OpenGLESRenderer::add_line_to_batch(const math::Vec2& start, const math::Vec2& end,
                                          uint32_t color) {
    uint8_t a = (color >> 24) & 0xFF;
    uint8_t b = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t r = color & 0xFF;
    uint32_t rgba = (r << 0) | (g << 8) | (b << 16) | (a << 24);
    
    batch_vertices_.push_back({{start.x, start.y}, {0, 0}, rgba});
    batch_vertices_.push_back({{end.x, end.y}, {0, 0}, rgba});
}

} // namespace diag::renderer

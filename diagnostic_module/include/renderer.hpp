#pragma once

#include "math_ops.hpp"
#include "memory_layout.hpp"
#include <cstdint>
#include <array>

// Minimal ImGui forward declarations
struct ImDrawList;
struct ImVec2;

namespace dm::render {

// 2D screen coordinate with depth info
struct screen_pos_t {
    float x, y;
    float depth;  // For occlusion testing
    bool visible;
};

// World to screen projection using view-projection matrix
[[nodiscard]] inline screen_pos_t world_to_screen(const math::vec3_t& world_pos,
                                                   const math::mat4x4_t& view_proj,
                                                   int screen_w, int screen_h) {
    math::vec4_t clip = view_proj.transform({world_pos.x, world_pos.y, world_pos.z, 1.0f});
    
    if (clip.w < 0.001f) {
        return {0, 0, 0, false};
    }
    
    // Perspective divide
    float ndc_x = clip.x / clip.w;
    float ndc_y = clip.y / clip.w;
    
    // NDC to screen space
    float screen_x = (ndc_x + 1.0f) * 0.5f * screen_w;
    float screen_y = (1.0f - ndc_y) * 0.5f * screen_h;  // Flip Y for OpenGL
    
    // Check if within screen bounds with margin
    bool visible = (ndc_x >= -1.0f && ndc_x <= 1.0f && 
                    ndc_y >= -1.0f && ndc_y <= 1.0f &&
                    clip.w > 0.1f);
    
    return {screen_x, screen_y, clip.w, visible};
}

// FOV cone test - check if point is within camera FOV
[[nodiscard]] inline bool in_fov(const math::vec3_t& target_pos,
                                  const math::vec3_t& camera_pos,
                                  const math::vec3_t& camera_forward,
                                  float fov_degrees) {
    math::vec3_t direction = (target_pos - camera_pos).normalized();
    float dot = camera_forward.dot(direction);
    float fov_cos = std::cos(fov_degrees * 0.5f * 0.01745329252f);
    return dot >= fov_cos;
}

// Distance-based alpha for fading distant objects
[[nodiscard]] inline uint32_t distance_alpha(float distance, float max_dist) {
    float alpha = 1.0f - std::min(distance / max_dist, 1.0f);
    return static_cast<uint32_t>(alpha * 255.0f) << 24;
}

// Health to color mapping
[[nodiscard]] inline uint32_t health_color(float health, float max_health) {
    float ratio = health / max_health;
    uint8_t r = static_cast<uint8_t>((1.0f - ratio) * 255.0f);
    uint8_t g = static_cast<uint8_t>(ratio * 255.0f);
    uint8_t b = 50;
    return 0xFF000000 | (b << 16) | (g << 8) | r;
}

// Bounding box corners for 3D box rendering
struct bbox_corners_t {
    std::array<math::vec3_t, 8> corners;
    
    void from_bounds(const math::bounds_t& bounds) {
        corners[0] = {bounds.min.x, bounds.min.y, bounds.min.z};
        corners[1] = {bounds.max.x, bounds.min.y, bounds.min.z};
        corners[2] = {bounds.max.x, bounds.max.y, bounds.min.z};
        corners[3] = {bounds.min.x, bounds.max.y, bounds.min.z};
        corners[4] = {bounds.min.x, bounds.min.y, bounds.max.z};
        corners[5] = {bounds.max.x, bounds.min.y, bounds.max.z};
        corners[6] = {bounds.max.x, bounds.max.y, bounds.max.z};
        corners[7] = {bounds.min.x, bounds.max.y, bounds.max.z};
    }
};

// Drawing context for ImGui overlay
class draw_context {
public:
    ImDrawList* draw_list = nullptr;
    int screen_w = 0;
    int screen_h = 0;
    
    void line(const math::vec2_t& from, const math::vec2_t& to, uint32_t color, float thickness);
    void rect(const math::vec2_t& min, const math::vec2_t& max, uint32_t color, float thickness);
    void rect_filled(const math::vec2_t& min, const math::vec2_t& max, uint32_t color);
    void circle(const math::vec2_t& center, float radius, uint32_t color, int segments);
    void circle_filled(const math::vec2_t& center, float radius, uint32_t color, int segments);
    void text(const math::vec2_t& pos, uint32_t color, const char* text);
    
    // 3D box projection
    void bbox_3d(const bbox_corners_t& corners, const math::mat4x4_t& view_proj, 
                 uint32_t color, float thickness);
    
    // Skeleton lines
    void skeleton(const std::array<screen_pos_t, game_offsets_t::BONE_COUNT>& bones,
                  uint32_t color, float thickness);
};

// Visibility check (simplified ray approximation)
[[nodiscard]] bool is_visible(const math::vec3_t& from, const math::vec3_t& to,
                               const render_context_t& ctx);

} // namespace dm::render

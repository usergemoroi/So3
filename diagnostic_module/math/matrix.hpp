#pragma once

#include "vector.hpp"
#include <cstring>

namespace diag::math {

// 4x4 matrix for view/projection transformations
struct Matrix4x4 {
    float m[4][4];
    
    constexpr Matrix4x4() {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }
    
    [[nodiscard]] static Matrix4x4 perspective(float fov, float aspect, float near_plane, float far_plane) {
        Matrix4x4 mat;
        const float tan_half_fov = std::tan(fov * 0.5f);
        
        mat.m[0][0] = 1.0f / (aspect * tan_half_fov);
        mat.m[1][1] = 1.0f / tan_half_fov;
        mat.m[2][2] = -(far_plane + near_plane) / (far_plane - near_plane);
        mat.m[2][3] = -1.0f;
        mat.m[3][2] = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);
        mat.m[3][3] = 0.0f;
        
        return mat;
    }
    
    [[nodiscard]] static Matrix4x4 look_at(const Vec3& eye, const Vec3& center, const Vec3& world_up) {
        Matrix4x4 mat;
        
        Vec3 f = (center - eye).normalized();
        Vec3 s = f.cross(world_up).normalized();
        Vec3 u = s.cross(f);
        
        mat.m[0][0] = s.x;
        mat.m[0][1] = u.x;
        mat.m[0][2] = -f.x;
        mat.m[0][3] = 0.0f;
        
        mat.m[1][0] = s.y;
        mat.m[1][1] = u.y;
        mat.m[1][2] = -f.y;
        mat.m[1][3] = 0.0f;
        
        mat.m[2][0] = s.z;
        mat.m[2][1] = u.z;
        mat.m[2][2] = -f.z;
        mat.m[2][3] = 0.0f;
        
        mat.m[3][0] = -s.dot(eye);
        mat.m[3][1] = -u.dot(eye);
        mat.m[3][2] = f.dot(eye);
        mat.m[3][3] = 1.0f;
        
        return mat;
    }
    
    [[nodiscard]] Matrix4x4 operator*(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }
    
    [[nodiscard]] Vec4 transform(const Vec4& v) const {
        return Vec4(
            m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w,
            m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w,
            m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w,
            m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w
        );
    }
    
    [[nodiscard]] Vec3 transform_point(const Vec3& p) const {
        Vec4 v = transform(Vec4(p.x, p.y, p.z, 1.0f));
        return v.to_cartesian();
    }
    
    [[nodiscard]] Vec3 transform_direction(const Vec3& d) const {
        Vec4 v = transform(Vec4(d.x, d.y, d.z, 0.0f));
        return Vec3(v.x, v.y, v.z);
    }
};

// View matrix extracted from game memory
struct ViewData {
    Matrix4x4 view_matrix;
    Matrix4x4 projection_matrix;
    Matrix4x4 view_projection;  // Cached multiplication
    Vec3 camera_position;
    float fov;
    float near_plane;
    float far_plane;
    int screen_width;
    int screen_height;
    
    void update_view_projection() {
        view_projection = projection_matrix * view_matrix;
    }
    
    // Convert world position to screen coordinates
    // Returns true if point is in front of camera and on screen
    [[nodiscard]] bool world_to_screen(const Vec3& world_pos, Vec2& out_screen) const {
        Vec4 clip = view_projection.transform(Vec4(world_pos.x, world_pos.y, world_pos.z, 1.0f));
        
        // Behind camera check
        if (clip.w < 0.001f) return false;
        
        // Perspective divide
        float ndc_x = clip.x / clip.w;
        float ndc_y = clip.y / clip.w;
        
        // NDC to screen coordinates
        out_screen.x = (ndc_x + 1.0f) * 0.5f * screen_width;
        out_screen.y = (1.0f - ndc_y) * 0.5f * screen_height;  // Flip Y
        
        // Check bounds with small margin
        const float margin = 50.0f;
        return out_screen.x >= -margin && 
               out_screen.x <= screen_width + margin &&
               out_screen.y >= -margin && 
               out_screen.y <= screen_height + margin;
    }
    
    // Calculate angle between camera forward and world direction
    [[nodiscard]] float get_fov_angle(const Vec3& world_pos) const {
        Vec3 dir = (world_pos - camera_position).normalized();
        Vec3 forward = -Vec3(
            view_matrix.m[0][2],
            view_matrix.m[1][2],
            view_matrix.m[2][2]
        );
        float dot_product = forward.dot(dir);
        return std::acos(std::clamp(dot_product, -1.0f, 1.0f));
    }
    
    // Check if world position is within camera FOV cone
    [[nodiscard]] bool in_fov(const Vec3& world_pos, float max_fov_deg = 90.0f) const {
        float angle_rad = get_fov_angle(world_pos);
        return angle_rad < (max_fov_deg * 3.14159265f / 180.0f);
    }
    
    // Get forward vector from view matrix
    [[nodiscard]] Vec3 get_forward() const {
        return Vec3(-view_matrix.m[0][2], -view_matrix.m[1][2], -view_matrix.m[2][2]);
    }
    
    // Get right vector from view matrix
    [[nodiscard]] Vec3 get_right() const {
        return Vec3(view_matrix.m[0][0], view_matrix.m[1][0], view_matrix.m[2][0]);
    }
    
    // Get up vector from view matrix
    [[nodiscard]] Vec3 get_up() const {
        return Vec3(view_matrix.m[0][1], view_matrix.m[1][1], view_matrix.m[2][1]);
    }
};

// Utility for 3D bounding box projection
struct BoundingBox {
    Vec3 min;
    Vec3 max;
    
    [[nodiscard]] Vec3 center() const {
        return (min + max) * 0.5f;
    }
    
    [[nodiscard]] Vec3 size() const {
        return max - min;
    }
    
    // Get 8 corners of the box
    [[nodiscard]] std::array<Vec3, 8> get_corners() const {
        return std::array<Vec3, 8>{
            Vec3(min.x, min.y, min.z),
            Vec3(max.x, min.y, min.z),
            Vec3(max.x, max.y, min.z),
            Vec3(min.x, max.y, min.z),
            Vec3(min.x, min.y, max.z),
            Vec3(max.x, min.y, max.z),
            Vec3(max.x, max.y, max.z),
            Vec3(min.x, max.y, max.z)
        };
    }
    
    // Project to screen space, returns true if any part is visible
    [[nodiscard]] bool project_to_screen(const ViewData& view, 
                                         std::array<Vec2, 8>& out_corners,
                                         Vec2& out_min, Vec2& out_max) const {
        auto world_corners = get_corners();
        bool any_visible = false;
        
        out_min = Vec2(static_cast<float>(view.screen_width), 
                       static_cast<float>(view.screen_height));
        out_max = Vec2(0.0f, 0.0f);
        
        for (int i = 0; i < 8; ++i) {
            if (view.world_to_screen(world_corners[i], out_corners[i])) {
                any_visible = true;
                out_min.x = std::min(out_min.x, out_corners[i].x);
                out_min.y = std::min(out_min.y, out_corners[i].y);
                out_max.x = std::max(out_max.x, out_corners[i].x);
                out_max.y = std::max(out_max.y, out_corners[i].y);
            }
        }
        
        return any_visible;
    }
};

} // namespace diag::math

#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>

namespace dm::math {

// SIMD-friendly vector types (NEON on ARM64)
struct vec3_t {
    float x, y, z;
    
    constexpr vec3_t() : x(0), y(0), z(0) {}
    constexpr vec3_t(float x, float y, float z) : x(x), y(y), z(z) {}
    
    [[nodiscard]] constexpr vec3_t operator+(const vec3_t& o) const {
        return {x + o.x, y + o.y, z + o.z};
    }
    
    [[nodiscard]] constexpr vec3_t operator-(const vec3_t& o) const {
        return {x - o.x, y - o.y, z - o.z};
    }
    
    [[nodiscard]] constexpr vec3_t operator*(float s) const {
        return {x * s, y * s, z * s};
    }
    
    [[nodiscard]] float dot(const vec3_t& o) const {
        return x * o.x + y * o.y + z * o.z;
    }
    
    [[nodiscard]] float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    [[nodiscard]] vec3_t normalized() const {
        float len = length();
        if (len > 0.0001f) {
            return (*this) * (1.0f / len);
        }
        return {0, 0, 0};
    }
    
    // Velocity prediction for aim compensation
    [[nodiscard]] vec3_t predict(float delta_time) const {
        return {x * delta_time, y * delta_time, z * delta_time};
    }
};

struct vec2_t {
    float x, y;
    
    constexpr vec2_t() : x(0), y(0) {}
    constexpr vec2_t(float x, float y) : x(x), y(y) {}
    
    [[nodiscard]] float distance(const vec2_t& o) const {
        float dx = x - o.x;
        float dy = y - o.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    [[nodiscard]] float length() const {
        return std::sqrt(x * x + y * y);
    }
};

struct vec4_t {
    float x, y, z, w;
    
    constexpr vec4_t() : x(0), y(0), z(0), w(0) {}
    constexpr vec4_t(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

// 4x4 matrix (column-major for OpenGL ES compatibility)
struct mat4x4_t {
    float m[16];
    
    [[nodiscard]] vec4_t transform(const vec4_t& v) const {
        return {
            m[0] * v.x + m[4] * v.y + m[8]  * v.z + m[12] * v.w,
            m[1] * v.x + m[5] * v.y + m[9]  * v.z + m[13] * v.w,
            m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w,
            m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w
        };
    }
    
    // Extract view direction from view matrix
    [[nodiscard]] vec3_t forward() const {
        return {-m[2], -m[6], -m[10]};
    }
    
    [[nodiscard]] vec3_t right() const {
        return {m[0], m[4], m[8]};
    }
    
    [[nodiscard]] vec3_t up() const {
        return {m[1], m[5], m[9]};
    }
};

struct bounds_t {
    vec3_t min;
    vec3_t max;
    
    [[nodiscard]] vec3_t center() const {
        return {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f, (min.z + max.z) * 0.5f};
    }
    
    [[nodiscard]] vec3_t size() const {
        return {max.x - min.x, max.y - min.y, max.z - min.z};
    }
};

// Euler angles in degrees (pitch, yaw, roll)
struct euler_t {
    float pitch, yaw, roll;
    
    [[nodiscard]] vec3_t to_direction() const {
        float pitch_rad = pitch * 0.01745329252f;
        float yaw_rad = yaw * 0.01745329252f;
        
        return {
            std::cos(pitch_rad) * std::cos(yaw_rad),
            std::cos(pitch_rad) * std::sin(yaw_rad),
            std::sin(pitch_rad)
        };
    }
    
    // Calculate angles from direction vector
    [[nodiscard]] static euler_t from_direction(const vec3_t& dir) {
        float yaw = std::atan2(dir.y, dir.x) * 57.29577951f;
        float pitch = std::asin(dir.z / dir.length()) * 57.29577951f;
        return {pitch, yaw, 0};
    }
};

// Bezier curve for smooth interpolation
[[nodiscard]] inline float bezier_smooth(float t, float p0, float p1, float p2, float p3) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    
    return uuu * p0 + 3.0f * uu * t * p1 + 3.0f * u * tt * p2 + ttt * p3;
}

// Exponential decay smoothing
[[nodiscard]] inline float exp_smooth(float current, float target, float factor, float dt) {
    return current + (target - current) * (1.0f - std::exp(-factor * dt));
}

// Clamp angle to [-180, 180] range
[[nodiscard]] inline float normalize_angle(float angle) {
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

// Calculate angle difference
[[nodiscard]] inline float angle_delta(float from, float to) {
    return normalize_angle(to - from);
}

} // namespace dm::math

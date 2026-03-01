#pragma once

#include <cmath>
#include <algorithm>

namespace diag::math {

struct Vec2 {
    float x, y;
    
    constexpr Vec2() : x(0.0f), y(0.0f) {}
    constexpr Vec2(float x, float y) : x(x), y(y) {}
    
    [[nodiscard]] constexpr float length() const {
        return std::sqrt(x * x + y * y);
    }
    
    [[nodiscard]] constexpr float length_sqr() const {
        return x * x + y * y;
    }
    
    [[nodiscard]] constexpr Vec2 normalized() const {
        const float len = length();
        if (len < 1e-6f) return Vec2(0.0f, 0.0f);
        return Vec2(x / len, y / len);
    }
    
    [[nodiscard]] constexpr Vec2 operator+(const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }
    
    [[nodiscard]] constexpr Vec2 operator-(const Vec2& other) const {
        return Vec2(x - other.x, y - other.y);
    }
    
    [[nodiscard]] constexpr Vec2 operator*(float scalar) const {
        return Vec2(x * scalar, y * scalar);
    }
    
    constexpr Vec2& operator+=(const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
};

struct Vec3 {
    float x, y, z;
    
    constexpr Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    [[nodiscard]] constexpr float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    [[nodiscard]] constexpr float length_sqr() const {
        return x * x + y * y + z * z;
    }
    
    [[nodiscard]] constexpr Vec3 normalized() const {
        const float len = length();
        if (len < 1e-6f) return Vec3(0.0f, 0.0f, 0.0f);
        return Vec3(x / len, y / len, z / len);
    }
    
    [[nodiscard]] constexpr Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    
    [[nodiscard]] constexpr Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    
    [[nodiscard]] constexpr Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
    
    [[nodiscard]] constexpr float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
    [[nodiscard]] constexpr Vec3 cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
    
    [[nodiscard]] constexpr float distance(const Vec3& other) const {
        return (*this - other).length();
    }
};

struct Vec4 {
    float x, y, z, w;
    
    constexpr Vec4() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    constexpr Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    
    [[nodiscard]] constexpr Vec3 xyz() const {
        return Vec3(x, y, z);
    }
    
    [[nodiscard]] constexpr Vec3 to_cartesian() const {
        if (std::abs(w) < 1e-6f) return Vec3(x, y, z);
        return Vec3(x / w, y / w, z / w);
    }
};

// Quaternion for smooth rotations
struct Quaternion {
    float x, y, z, w;
    
    [[nodiscard]] static Quaternion from_euler(float pitch, float yaw, float roll) {
        const float cy = std::cos(yaw * 0.5f);
        const float sy = std::sin(yaw * 0.5f);
        const float cp = std::cos(pitch * 0.5f);
        const float sp = std::sin(pitch * 0.5f);
        const float cr = std::cos(roll * 0.5f);
        const float sr = std::sin(roll * 0.5f);
        
        return {
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        };
    }
    
    [[nodiscard]] Vec3 to_euler() const {
        Vec3 angles;
        
        // Roll (X-axis rotation)
        float sinr_cosp = 2.0f * (w * x + y * z);
        float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
        angles.x = std::atan2(sinr_cosp, cosr_cosp);
        
        // Pitch (Y-axis rotation)
        float sinp = 2.0f * (w * y - z * x);
        if (std::abs(sinp) >= 1.0f)
            angles.y = std::copysign(3.14159265f / 2.0f, sinp);
        else
            angles.y = std::asin(sinp);
        
        // Yaw (Z-axis rotation)
        float siny_cosp = 2.0f * (w * z + x * y);
        float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
        angles.z = std::atan2(siny_cosp, cosy_cosp);
        
        return angles;
    }
    
    [[nodiscard]] Quaternion slerp(const Quaternion& other, float t) const {
        float dot = x * other.x + y * other.y + z * other.z + w * other.w;
        
        if (dot < 0.0f) {
            dot = -dot;
            Quaternion neg = {-other.x, -other.y, -other.z, -other.w};
            return slerp(neg, t);
        }
        
        if (dot > 0.9995f) {
            Quaternion result = {
                x + t * (other.x - x),
                y + t * (other.y - y),
                z + t * (other.z - z),
                w + t * (other.w - w)
            };
            // Normalize
            float len = std::sqrt(result.x * result.x + result.y * result.y + 
                                  result.z * result.z + result.w * result.w);
            return {result.x / len, result.y / len, result.z / len, result.w / len};
        }
        
        float theta0 = std::acos(dot);
        float theta = theta0 * t;
        float sin_theta = std::sin(theta);
        float sin_theta0 = std::sin(theta0);
        
        float s0 = std::cos(theta) - dot * sin_theta / sin_theta0;
        float s1 = sin_theta / sin_theta0;
        
        return {
            x * s0 + other.x * s1,
            y * s0 + other.y * s1,
            z * s0 + other.z * s1,
            w * s0 + other.w * s1
        };
    }
};

} // namespace diag::math

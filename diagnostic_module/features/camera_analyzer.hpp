#pragma once

#include "../math/vector.hpp"
#include "../core/entity_manager.hpp"
#include <functional>
#include <chrono>

namespace diag::features {

// Smoothing algorithm types
enum class SmoothingType {
    None,           // Instant
    Linear,         // Constant speed
    Exponential,    // Exponential decay (natural feel)
    BezierCubic,    // Cubic bezier curve
    EaseOutBack,    // Overshoot and settle
    Spring          // Physics-based spring
};

// Configuration for camera analysis
struct CameraAnalyzerConfig {
    // FOV filtering
    float max_fov_degrees = 90.0f;
    float fov_threshold = 60.0f;  // Soft threshold for priority
    
    // Smoothing parameters
    SmoothingType smoothing_type = SmoothingType::Exponential;
    float smoothing_factor = 0.15f;  // 0-1, lower = smoother
    float max_rotation_speed = 360.0f;  // Degrees per second
    
    // Bone priority
    core::BoneIndex preferred_bone = core::BoneIndex::Head;
    core::BoneIndex fallback_bone = core::BoneIndex::Spine2;
    
    // Velocity prediction
    bool use_prediction = true;
    float prediction_factor = 0.05f;  // Seconds to predict ahead
    
    // Distance limits
    float min_target_distance = 1.0f;
    float max_target_distance = 500.0f;
    
    // Human-like variation
    float jitter_amount = 0.5f;  // Subtle position jitter
    float reaction_delay_ms = 50.0f;  // Simulated reaction time
};

// Target information for tracking
struct TargetInfo {
    const core::EntityInfo* entity;
    core::BoneIndex bone;
    math::Vec3 world_position;
    math::Vec2 screen_position;
    float distance;
    float fov_angle;
    float priority_score;
    bool visible;
};

// Smooth rotation state
struct RotationState {
    math::Vec2 current_angles;      // Pitch, Yaw in degrees
    math::Vec2 target_angles;
    math::Vec2 velocity;            // Angular velocity
    math::Vec2 acceleration;        // For spring physics
    
    std::chrono::steady_clock::time_point last_update;
    float delta_time;
    
    void update_timing() {
        auto now = std::chrono::steady_clock::now();
        delta_time = std::chrono::duration<float>(now - last_update).count();
        last_update = now;
    }
};

// Camera analysis and smooth aiming system
class CameraAnalyzer {
public:
    static CameraAnalyzer& instance() {
        static CameraAnalyzer inst;
        return inst;
    }
    
    void initialize(const CameraAnalyzerConfig& config);
    
    // Update with current view data
    void update_view(const math::ViewData& view);
    
    // Find best target based on configuration
    [[nodiscard]] std::optional<TargetInfo> select_target(
        const std::vector<core::EntityInfo>& entities
    );
    
    // Calculate rotation angles to target
    [[nodiscard]] math::Vec2 calculate_angles(const math::Vec3& target_pos) const;
    
    // Apply smoothing to rotation
    [[nodiscard]] math::Vec2 smooth_rotation(
        const math::Vec2& current,
        const math::Vec2& target,
        float delta_time
    );
    
    // Bezier curve smoothing (cubic)
    [[nodiscard]] math::Vec2 bezier_smooth(
        const math::Vec2& p0,  // Start
        const math::Vec2& p1,  // Control 1
        const math::Vec2& p2,  // Control 2
        const math::Vec2& p3,  // End
        float t
    ) const;
    
    // Spring physics update
    void update_spring(RotationState& state, const math::Vec2& target);
    
    // Main update - call each frame
    void process_frame();
    
    // Get current state for visualization
    [[nodiscard]] const RotationState& get_state() const { return state_; }
    [[nodiscard]] const std::optional<TargetInfo>& get_current_target() const { 
        return current_target_; 
    }
    
    // Manual override for testing
    void set_manual_target(const math::Vec3& world_pos);
    void clear_manual_target();
    
    // Config access
    void set_config(const CameraAnalyzerConfig& config) { config_ = config; }
    [[nodiscard]] const CameraAnalyzerConfig& get_config() const { return config_; }
    
    // Latency/jitter analysis
    struct LatencyMetrics {
        float avg_input_delay_ms;
        float avg_render_delay_ms;
        float jitter_ms;
        int dropped_frames;
    };
    
    [[nodiscard]] LatencyMetrics get_latency_metrics() const { return metrics_; }

private:
    CameraAnalyzer() = default;
    
    [[nodiscard]] float calculate_target_score(const TargetInfo& target) const;
    [[nodiscard]] math::Vec3 predict_position(
        const core::EntityInfo& entity,
        float delta_time
    ) const;
    [[nodiscard]] float get_smoothing_factor(float distance_ratio) const;
    
    // Easing functions
    [[nodiscard]] float ease_out_cubic(float t) const;
    [[nodiscard]] float ease_out_back(float t) const;
    
    CameraAnalyzerConfig config_;
    math::ViewData current_view_;
    RotationState state_;
    std::optional<TargetInfo> current_target_;
    std::optional<math::Vec3> manual_target_;
    LatencyMetrics metrics_;
    
    // Timing for jitter analysis
    std::vector<float> frame_times_;
    static constexpr size_t MAX_FRAME_SAMPLES = 120;
    
    // Bezier control points (for organic movement)
    math::Vec2 control_point_1_;
    math::Vec2 control_point_2_;
    float bezier_progress_ = 0.0f;
};

// Utility: Convert Euler angles to direction vector
[[nodiscard]] inline math::Vec3 angles_to_direction(float pitch_deg, float yaw_deg) {
    float pitch_rad = pitch_deg * 3.14159265f / 180.0f;
    float yaw_rad = yaw_deg * 3.14159265f / 180.0f;
    
    return math::Vec3(
        std::cos(pitch_rad) * std::cos(yaw_rad),
        std::sin(pitch_rad),
        std::cos(pitch_rad) * std::sin(yaw_rad)
    );
}

// Utility: Convert direction to Euler angles
[[nodiscard]] inline math::Vec2 direction_to_angles(const math::Vec3& dir) {
    float yaw = std::atan2(dir.z, dir.x) * 180.0f / 3.14159265f;
    float pitch = std::asin(dir.y / dir.length()) * 180.0f / 3.14159265f;
    return math::Vec2(pitch, yaw);
}

// Clamp angles to valid ranges
[[nodiscard]] inline math::Vec2 clamp_angles(const math::Vec2& angles) {
    math::Vec2 result = angles;
    
    // Pitch: -90 to +90
    result.x = std::clamp(result.x, -89.0f, 89.0f);
    
    // Yaw: -180 to +180 (normalize)
    while (result.y > 180.0f) result.y -= 360.0f;
    while (result.y < -180.0f) result.y += 360.0f;
    
    return result;
}

// Calculate angle difference, handling wraparound
[[nodiscard]] inline math::Vec2 angle_delta(const math::Vec2& from, const math::Vec2& to) {
    math::Vec2 delta = to - from;
    
    // Handle yaw wraparound
    if (delta.y > 180.0f) delta.y -= 360.0f;
    if (delta.y < -180.0f) delta.y += 360.0f;
    
    return delta;
}

} // namespace diag::features

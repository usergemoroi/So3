#pragma once

#include "math_ops.hpp"
#include "memory_layout.hpp"
#include "secure_types.hpp"
#include <cstdint>
#include <chrono>

namespace dm::aim {

// Smoothing configuration
struct smooth_config_t {
    // Exponential smoothing factor (higher = faster)
    float exp_factor = 8.0f;
    
    // Bezier control points for custom curve
    float bezier_p1 = 0.0f;
    float bezier_p2 = 1.0f;
    
    // Human-like variation parameters
    float jitter_amount = 0.5f;      // Micro-movements
    float overshoot_amount = 0.1f;   // Slight overshoot before settling
    float reaction_delay = 0.05f;    // Initial reaction time
    
    // FOV limits
    float max_fov_horizontal = 90.0f;
    float max_fov_vertical = 60.0f;
    
    // Distance scaling
    float close_range_mult = 1.2f;   // Faster at close range
    float long_range_mult = 0.8f;    // Slower at long range
};

// Target selection criteria
struct target_filter_t {
    float max_distance = 500.0f;
    float min_health = 1.0f;
    bool ignore_team = false;
    int32_t local_team = -1;
    bool require_visible = true;
    float priority_distance_weight = 0.3f;
    float priority_fov_weight = 0.7f;
};

// Target info for evaluation
struct target_info_t {
    uintptr_t entity_ptr;
    math::vec3_t position;
    math::vec3_t predicted_position;
    math::vec3_t velocity;
    float health;
    int32_t team;
    float distance;
    float fov_angle;
    bool visible;
    game_offsets_t::bone_idx best_bone;
    float score;
};

// Internal state for smooth transition
struct aim_state_t {
    math::euler_t current_angles;
    math::euler_t target_angles;
    math::euler_t last_angles;
    
    float progress = 0.0f;
    float reaction_timer = 0.0f;
    bool is_tracking = false;
    
    uint64_t last_update_ms = 0;
    target_info_t current_target;
    
    // Per-axis smoothing multipliers for human-like feel
    float pitch_mult = 1.0f;
    float yaw_mult = 1.0f;
    
    void reset() {
        is_tracking = false;
        progress = 0.0f;
        reaction_timer = 0.0f;
    }
};

// Main aim system
class aim_controller {
public:
    smooth_config_t config;
    target_filter_t filter;
    
    // Calculate best bone position for given entity
    [[nodiscard]] math::vec3_t get_best_bone(const entity_cache_t& entity,
                                              const math::vec3_t& view_pos,
                                              const math::vec3_t& view_fwd) const;
    
    // Evaluate target score based on distance, FOV, visibility
    [[nodiscard]] float evaluate_target(const target_info_t& target) const;
    
    // Select best target from entity list
    [[nodiscard]] target_info_t select_target(const entity_cache_t* entities,
                                               size_t count,
                                               const math::vec3_t& view_pos,
                                               const math::euler_t& view_angles) const;
    
    // Update aim state for smooth transition
    void update(aim_state_t& state, const target_info_t& target, float dt);
    
    // Calculate final smoothed angles
    [[nodiscard]] math::euler_t calculate_smoothed_angles(const aim_state_t& state,
                                                           float dt) const;
    
    // Apply human-like micro-jitter
    void apply_jitter(math::euler_t& angles, uint64_t time_ms);
    
    // Velocity compensation
    [[nodiscard]] math::vec3_t compensate_velocity(const math::vec3_t& target_pos,
                                                    const math::vec3_t& target_vel,
                                                    float bullet_speed,
                                                    float distance) const;
private:
    [[nodiscard]] float bezier_interp(float t) const;
    [[nodiscard]] float get_distance_multiplier(float distance) const;
};

// Utility: Check if angles are within FOV limits
[[nodiscard]] bool within_fov_limits(const math::euler_t& current,
                                      const math::euler_t& target,
                                      float max_h_fov,
                                      float max_v_fov);

// Utility: Calculate angle to target
[[nodiscard]] math::euler_t angles_to_target(const math::vec3_t& from,
                                              const math::vec3_t& to);

} // namespace dm::aim

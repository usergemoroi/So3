#include "aim_system.hpp"
#include "secure_types.hpp"
#include <cmath>
#include <algorithm>

namespace dm::aim {

math::vec3_t aim_controller::get_best_bone(const entity_cache_t& entity,
                                           const math::vec3_t& view_pos,
                                           const math::vec3_t& view_fwd) const {
    float best_score = -1.0f;
    game_offsets_t::bone_idx best_bone = game_offsets_t::BONE_HEAD;
    
    for (int i = 0; i < game_offsets_t::BONE_COUNT; ++i) {
        auto bone = static_cast<game_offsets_t::bone_idx>(i);
        const auto& pos = entity.bone_positions[i];
        
        // Calculate direction to bone
        math::vec3_t dir = (pos - view_pos).normalized();
        
        // Score based on priority and alignment with view
        float alignment = view_fwd.dot(dir);
        float priority = game_offsets_t::bone_priority(bone);
        float score = alignment * priority;
        
        if (score > best_score) {
            best_score = score;
            best_bone = bone;
        }
    }
    
    return entity.bone_positions[best_bone];
}

float aim_controller::evaluate_target(const target_info_t& target) const {
    // Skip invalid targets
    if (target.health < filter.min_health) return 0.0f;
    if (target.distance > filter.max_distance) return 0.0f;
    if (target.team == filter.local_team && !filter.ignore_team) return 0.0f;
    if (filter.require_visible && !target.visible) return 0.0f;
    
    // Distance score (closer is better, non-linear)
    float dist_score = 1.0f - (target.distance / filter.max_distance);
    dist_score = dist_score * dist_score;  // Quadratic falloff
    
    // FOV score (smaller angle is better)
    float fov_score = 1.0f - std::min(target.fov_angle / 90.0f, 1.0f);
    fov_score = fov_score * fov_score * fov_score;  // Cubic falloff for sharp preference
    
    // Combined score with weights
    float score = dist_score * filter.priority_distance_weight +
                  fov_score * filter.priority_fov_weight;
    
    // Bone priority multiplier
    score *= game_offsets_t::bone_priority(target.best_bone);
    
    return score;
}

target_info_t aim_controller::select_target(const entity_cache_t* entities,
                                            size_t count,
                                            const math::vec3_t& view_pos,
                                            const math::euler_t& view_angles) const {
    target_info_t best_target{};
    best_target.entity_ptr = 0;
    best_target.score = 0.0f;
    
    math::vec3_t view_fwd = view_angles.to_direction();
    
    for (size_t i = 0; i < count; ++i) {
        const auto& entity = entities[i];
        if (!entity.valid) continue;
        
        target_info_t info;
        info.entity_ptr = reinterpret_cast<uintptr_t>(&entity);
        info.position = entity.position;
        info.velocity = entity.velocity;
        info.predicted_position = entity.position + entity.velocity * 0.05f;  // 50ms prediction
        info.health = entity.health;
        info.team = entity.team;
        info.distance = entity.position.distance(view_pos);
        
        // Calculate FOV angle
        math::vec3_t dir = (entity.position - view_pos).normalized();
        float dot = view_fwd.dot(dir);
        info.fov_angle = std::acos(std::clamp(dot, -1.0f, 1.0f)) * 57.29577951f;
        
        // Find best bone
        info.best_bone = game_offsets_t::BONE_HEAD;  // Simplified, should use get_best_bone
        
        // Visibility check
        info.visible = true;  // Should use actual visibility test
        
        // Evaluate
        info.score = evaluate_target(info);
        
        if (info.score > best_target.score) {
            best_target = info;
        }
    }
    
    return best_target;
}

void aim_controller::update(aim_state_t& state, const target_info_t& target, float dt) {
    // Calculate target angles
    math::euler_t target_angles = angles_to_target(
        math::vec3_t{},  // View position (relative to self)
        target.predicted_position
    );
    
    // Check if this is a new target
    if (state.current_target.entity_ptr != target.entity_ptr) {
        state.reset();
        state.current_target = target;
        state.last_angles = state.current_angles;
        state.reaction_timer = config_.reaction_delay;
        
        // Randomize smoothing multipliers for human-like variation
        state.pitch_mult = 0.8f + (reinterpret_cast<uintptr_t>(&state) % 40) / 100.0f;
        state.yaw_mult = 0.8f + (reinterpret_cast<uintptr_t>(&target) % 40) / 100.0f;
    }
    
    state.target_angles = target_angles;
    state.is_tracking = true;
    
    // Handle reaction delay
    if (state.reaction_timer > 0.0f) {
        state.reaction_timer -= dt;
        return;
    }
    
    // Update progress based on distance to target
    math::euler_t delta;
    delta.pitch = math::angle_delta(state.current_angles.pitch, target_angles.pitch);
    delta.yaw = math::angle_delta(state.current_angles.yaw, target_angles.yaw);
    
    float total_delta = std::sqrt(delta.pitch * delta.pitch + delta.yaw * delta.yaw);
    
    // Progress moves faster for larger angles (human-like)
    float progress_rate = 0.5f + (total_delta / 90.0f);
    state.progress = std::min(state.progress + dt * progress_rate, 1.0f);
}

math::euler_t aim_controller::calculate_smoothed_angles(const aim_state_t& state, float dt) const {
    if (!state.is_tracking) {
        return state.current_angles;
    }
    
    // Calculate angle deltas
    float delta_pitch = math::angle_delta(state.current_angles.pitch, state.target_angles.pitch);
    float delta_yaw = math::angle_delta(state.current_angles.yaw, state.target_angles.yaw);
    
    // Apply bezier smoothing to progress
    float smooth_t = bezier_interp(state.progress);
    
    // Calculate distance-based multiplier
    float dist_mult = get_distance_multiplier(state.current_target.distance);
    
    // Apply exponential smoothing with bezier curve
    float exp_factor = config_.exp_factor * dist_mult;
    
    // Smooth each axis independently
    float smooth_pitch = math::exp_smooth(
        0.0f, delta_pitch, exp_factor * state.pitch_mult, dt);
    float smooth_yaw = math::exp_smooth(
        0.0f, delta_yaw, exp_factor * state.yaw_mult, dt);
    
    // Apply bezier curve to the result
    smooth_pitch *= smooth_t;
    smooth_yaw *= smooth_t;
    
    // Calculate new angles
    math::euler_t result;
    result.pitch = math::normalize_angle(state.current_angles.pitch + smooth_pitch);
    result.yaw = math::normalize_angle(state.current_angles.yaw + smooth_yaw);
    result.roll = 0.0f;
    
    return result;
}

void aim_controller::apply_jitter(math::euler_t& angles, uint64_t time_ms) {
    // Deterministic jitter based on time
    uint64_t seed = time_ms * 6364136223846793005ULL + 1442695040888963407ULL;
    
    float jitter_x = (static_cast<float>(seed & 0xFFFF) / 65535.0f - 0.5f) * config_.jitter_amount;
    float jitter_y = (static_cast<float>((seed >> 16) & 0xFFFF) / 65535.0f - 0.5f) * config_.jitter_amount;
    
    angles.pitch += jitter_x;
    angles.yaw += jitter_y;
}

math::vec3_t aim_controller::compensate_velocity(const math::vec3_t& target_pos,
                                                  const math::vec3_t& target_vel,
                                                  float bullet_speed,
                                                  float distance) const {
    // Simplified ballistics compensation
    if (bullet_speed <= 0.0f) return target_pos;
    
    float time_to_target = distance / bullet_speed;
    return target_pos + target_vel * time_to_target;
}

float aim_controller::bezier_interp(float t) const {
    return math::bezier_smooth(t, 0.0f, config_.bezier_p1, config_.bezier_p2, 1.0f);
}

float aim_controller::get_distance_multiplier(float distance) const {
    // Closer = faster aim, further = slower
    if (distance < 50.0f) {
        return config_.close_range_mult;
    } else if (distance > 200.0f) {
        return config_.long_range_mult;
    }
    // Linear interpolation between
    float t = (distance - 50.0f) / 150.0f;
    return config_.close_range_mult + t * (config_.long_range_mult - config_.close_range_mult);
}

bool within_fov_limits(const math::euler_t& current, const math::euler_t& target,
                       float max_h_fov, float max_v_fov) {
    float delta_yaw = std::abs(math::angle_delta(current.yaw, target.yaw));
    float delta_pitch = std::abs(math::angle_delta(current.pitch, target.pitch));
    
    return delta_yaw <= max_h_fov && delta_pitch <= max_v_fov;
}

math::euler_t angles_to_target(const math::vec3_t& from, const math::vec3_t& to) {
    math::vec3_t dir = (to - from).normalized();
    return math::euler_t::from_direction(dir);
}

} // namespace dm::aim

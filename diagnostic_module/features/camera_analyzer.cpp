#include "camera_analyzer.hpp"
#include "../core/memory.hpp"
#include <cmath>
#include <random>

namespace diag::features {

void CameraAnalyzer::initialize(const CameraAnalyzerConfig& config) {
    config_ = config;
    state_.last_update = std::chrono::steady_clock::now();
    state_.current_angles = math::Vec2(0.0f, 0.0f);
    state_.target_angles = math::Vec2(0.0f, 0.0f);
    state_.velocity = math::Vec2(0.0f, 0.0f);
    state_.acceleration = math::Vec2(0.0f, 0.0f);
    
    frame_times_.reserve(MAX_FRAME_SAMPLES);
}

void CameraAnalyzer::update_view(const math::ViewData& view) {
    current_view_ = view;
    
    // Extract current angles from view matrix
    math::Vec3 forward = view.get_forward();
    state_.current_angles = direction_to_angles(forward);
    
    state_.update_timing();
    
    // Record frame time for jitter analysis
    frame_times_.push_back(state_.delta_time * 1000.0f);  // Convert to ms
    if (frame_times_.size() > MAX_FRAME_SAMPLES) {
        frame_times_.erase(frame_times_.begin());
    }
}

std::optional<TargetInfo> CameraAnalyzer::select_target(
    const std::vector<core::EntityInfo>& entities
) {
    if (entities.empty()) {
        current_target_ = std::nullopt;
        return std::nullopt;
    }
    
    TargetInfo best_target;
    float best_score = -1.0f;
    
    for (const auto& entity : entities) {
        if (entity.state != core::EntityState::Alive) continue;
        if (entity.is_local_player) continue;
        
        // Distance check
        float distance = current_view_.camera_position.distance(entity.world_position);
        if (distance < config_.min_target_distance || distance > config_.max_target_distance) {
            continue;
        }
        
        // FOV check
        if (!current_view_.in_fov(entity.world_position, config_.max_fov_degrees)) {
            continue;
        }
        
        // Get target position with prediction
        math::Vec3 target_pos = entity.world_position;
        if (config_.use_prediction) {
            target_pos = predict_position(entity, config_.prediction_factor);
        }
        
        // Try preferred bone first
        math::Vec3 bone_pos = entity.get_bone_position(config_.preferred_bone);
        if (!entity.bones[static_cast<size_t>(config_.preferred_bone)].valid) {
            bone_pos = entity.get_bone_position(config_.fallback_bone);
        }
        
        // Add jitter for human-like behavior
        if (config_.jitter_amount > 0.0f) {
            static std::mt19937 rng(static_cast<unsigned>(
                std::chrono::steady_clock::now().time_since_epoch().count()
            ));
            std::uniform_real_distribution<float> dist(-config_.jitter_amount, config_.jitter_amount);
            bone_pos.x += dist(rng);
            bone_pos.y += dist(rng);
            bone_pos.z += dist(rng);
        }
        
        // Calculate FOV angle
        float fov_angle = current_view_.get_fov_angle(bone_pos) * 180.0f / 3.14159265f;
        
        // Project to screen
        math::Vec2 screen_pos;
        bool on_screen = current_view_.world_to_screen(bone_pos, screen_pos);
        
        // Calculate screen distance from center
        math::Vec2 center(current_view_.screen_width * 0.5f, current_view_.screen_height * 0.5f);
        float screen_dist = (screen_pos - center).length();
        
        TargetInfo info;
        info.entity = &entity;
        info.bone = entity.bones[static_cast<size_t>(config_.preferred_bone)].valid 
                    ? config_.preferred_bone 
                    : config_.fallback_bone;
        info.world_position = bone_pos;
        info.screen_position = screen_pos;
        info.distance = distance;
        info.fov_angle = fov_angle;
        info.visible = on_screen;
        
        float score = calculate_target_score(info);
        
        if (score > best_score) {
            best_score = score;
            best_target = info;
        }
    }
    
    if (best_score > 0.0f) {
        current_target_ = best_target;
        return best_target;
    }
    
    current_target_ = std::nullopt;
    return std::nullopt;
}

math::Vec2 CameraAnalyzer::calculate_angles(const math::Vec3& target_pos) const {
    math::Vec3 dir = target_pos - current_view_.camera_position;
    return direction_to_angles(dir);
}

math::Vec2 CameraAnalyzer::smooth_rotation(
    const math::Vec2& current,
    const math::Vec2& target,
    float delta_time
) {
    math::Vec2 delta = angle_delta(current, target);
    
    // Distance-based smoothing factor
    float distance_ratio = delta.length() / config_.max_fov_degrees;
    float smoothing = get_smoothing_factor(distance_ratio);
    
    switch (config_.smoothing_type) {
        case SmoothingType::None:
            return target;
            
        case SmoothingType::Linear: {
            float max_step = config_.max_rotation_speed * delta_time;
            float step = std::min(delta.length(), max_step);
            if (delta.length() > 0.0f) {
                math::Vec2 dir = delta.normalized();
                return clamp_angles(current + dir * step);
            }
            return current;
        }
        
        case SmoothingType::Exponential: {
            // Exponential decay: new = current + (target - current) * factor
            math::Vec2 new_angles = current + delta * smoothing;
            return clamp_angles(new_angles);
        }
        
        case SmoothingType::BezierCubic: {
            // Update bezier progress
            bezier_progress_ += delta_time * (1.0f - smoothing * 0.5f);
            if (bezier_progress_ > 1.0f) bezier_progress_ = 1.0f;
            
            // Calculate control points for organic curve
            control_point_1_ = current + delta * 0.3f;
            control_point_2_ = current + delta * 0.7f;
            
            return bezier_smooth(current, control_point_1_, control_point_2_, target, 
                                ease_out_cubic(bezier_progress_));
        }
        
        case SmoothingType::EaseOutBack: {
            bezier_progress_ += delta_time * 2.0f;
            if (bezier_progress_ > 1.0f) bezier_progress_ = 1.0f;
            
            float t = ease_out_back(bezier_progress_);
            return clamp_angles(current + delta * t);
        }
        
        case SmoothingType::Spring:
            // Handled by update_spring
            return current;
    }
    
    return current;
}

math::Vec2 CameraAnalyzer::bezier_smooth(
    const math::Vec2& p0,
    const math::Vec2& p1,
    const math::Vec2& p2,
    const math::Vec2& p3,
    float t
) const {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    
    math::Vec2 p = p0 * uuu;  // (1-t)^3 * P0
    p += p1 * (3.0f * uu * t);  // 3*(1-t)^2*t * P1
    p += p2 * (3.0f * u * tt);  // 3*(1-t)*t^2 * P2
    p += p3 * ttt;  // t^3 * P3
    
    return p;
}

void CameraAnalyzer::update_spring(RotationState& state, const math::Vec2& target) {
    // Spring constants
    const float stiffness = 200.0f;
    const float damping = 15.0f;
    const float mass = 1.0f;
    
    math::Vec2 displacement = angle_delta(state.current_angles, target);
    
    // Spring force: F = -k * x
    math::Vec2 spring_force = displacement * (-stiffness);
    
    // Damping force: F = -c * v
    math::Vec2 damping_force = state.velocity * (-damping);
    
    // Acceleration: a = F / m
    state.acceleration = (spring_force + damping_force) * (1.0f / mass);
    
    // Integrate
    state.velocity += state.acceleration * state.delta_time;
    state.current_angles += state.velocity * state.delta_time;
    state.current_angles = clamp_angles(state.current_angles);
}

void CameraAnalyzer::process_frame() {
    state_.update_timing();
    
    if (manual_target_) {
        state_.target_angles = calculate_angles(*manual_target_);
    } else if (current_target_) {
        // Update target position (with fresh prediction)
        math::Vec3 target_pos = current_target_->world_position;
        if (config_.use_prediction && current_target_->entity) {
            target_pos = predict_position(*current_target_->entity, config_.prediction_factor);
        }
        state_.target_angles = calculate_angles(target_pos);
    } else {
        // No target, maintain current angles
        return;
    }
    
    // Apply smoothing
    if (config_.smoothing_type == SmoothingType::Spring) {
        update_spring(state_, state_.target_angles);
    } else {
        state_.current_angles = smooth_rotation(
            state_.current_angles,
            state_.target_angles,
            state_.delta_time
        );
    }
    
    // Update metrics
    if (frame_times_.size() >= 10) {
        float sum = 0.0f;
        float sum_sq = 0.0f;
        for (float t : frame_times_) {
            sum += t;
            sum_sq += t * t;
        }
        float mean = sum / frame_times_.size();
        float variance = (sum_sq / frame_times_.size()) - (mean * mean);
        metrics_.jitter_ms = std::sqrt(variance);
        metrics_.avg_render_delay_ms = mean;
    }
}

void CameraAnalyzer::set_manual_target(const math::Vec3& world_pos) {
    manual_target_ = world_pos;
    bezier_progress_ = 0.0f;
}

void CameraAnalyzer::clear_manual_target() {
    manual_target_ = std::nullopt;
}

float CameraAnalyzer::calculate_target_score(const TargetInfo& target) const {
    // Normalize factors to 0-1 range
    float distance_score = 1.0f - (target.distance / config_.max_target_distance);
    float fov_score = 1.0f - (target.fov_angle / config_.max_fov_degrees);
    
    // Bone priority bonus
    float bone_bonus = static_cast<float>(core::get_bone_priority(target.bone)) / 100.0f;
    
    // Visibility bonus
    float visibility_bonus = target.visible ? 0.2f : 0.0f;
    
    // FOV threshold penalty
    float fov_penalty = 0.0f;
    if (target.fov_angle > config_.fov_threshold) {
        fov_penalty = (target.fov_angle - config_.fov_threshold) / config_.max_fov_degrees;
    }
    
    // Combined weighted score
    float score = distance_score * 0.3f + 
                  fov_score * 0.4f + 
                  bone_bonus * 0.2f + 
                  visibility_bonus - 
                  fov_penalty * 0.3f;
    
    return score;
}

math::Vec3 CameraAnalyzer::predict_position(const core::EntityInfo& entity, float delta_time) const {
    // Simple linear prediction: pos + vel * dt
    // More advanced: account for acceleration, gravity if applicable
    return entity.predict_position(delta_time);
}

float CameraAnalyzer::get_smoothing_factor(float distance_ratio) const {
    // Adaptive smoothing: faster when far from target, slower when close
    float adaptive = config_.smoothing_factor * (0.5f + distance_ratio * 0.5f);
    return std::clamp(adaptive, 0.05f, 0.5f);
}

float CameraAnalyzer::ease_out_cubic(float t) const {
    float u = 1.0f - t;
    return 1.0f - u * u * u;
}

float CameraAnalyzer::ease_out_back(float t) const {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    
    float u = t - 1.0f;
    return 1.0f + c3 * u * u * u + c1 * u * u;
}

} // namespace diag::features

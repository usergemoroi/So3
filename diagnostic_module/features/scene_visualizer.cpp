#include "scene_visualizer.hpp"
#include <algorithm>
#include <cmath>

namespace diag::features {

void SceneVisualizer::initialize(const VisualConfig& config) {
    config_ = config;
    
    // Pre-allocate buffers to avoid runtime allocations
    lines_.reserve(1024);
    boxes_.reserve(256);
    circles_.reserve(128);
    texts_.reserve(128);
}

void SceneVisualizer::prepare_frame(const std::vector<core::EntityInfo>& entities) {
    clear_primitives();
    
    if (!config_.enabled) return;
    
    int rendered_count = 0;
    
    for (const auto& entity : entities) {
        if (entity.state != core::EntityState::Alive) continue;
        
        // Distance culling
        float distance = current_view_.camera_position.distance(entity.world_position);
        if (distance > config_.max_render_distance) continue;
        
        // FOV culling
        if (!current_view_.in_fov(entity.world_position, 120.0f)) continue;
        
        // Check if on screen
        if (!entity.on_screen) continue;
        
        // Render based on config
        if (config_.show_2d_boxes) {
            render_entity_2d(entity);
        }
        
        if (config_.show_3d_boxes) {
            render_entity_3d(entity);
        }
        
        if (config_.show_skeleton) {
            render_skeleton(entity);
        }
        
        if (config_.show_health && entity.max_health > 0) {
            render_health_bar(entity, entity.screen_position);
        }
        
        if (config_.show_trajectory) {
            render_trajectory(entity);
        }
        
        rendered_count++;
    }
    
    stats_.entities_rendered = rendered_count;
    stats_.primitives_drawn = static_cast<int>(lines_.size() + boxes_.size() + 
                                               circles_.size() + texts_.size());
}

void SceneVisualizer::render_frame() {
    // This is called from the render hook context
    // Actual rendering is done by the renderer backend using these primitives
    // Implementation depends on graphics API (OpenGL/Vulkan)
}

void SceneVisualizer::clear_primitives() {
    lines_.clear();
    boxes_.clear();
    circles_.clear();
    texts_.clear();
}

void SceneVisualizer::add_line(const math::Vec2& start, const math::Vec2& end,
                               const Color& color, float thickness) {
    lines_.push_back({start, end, color, thickness});
}

void SceneVisualizer::add_box(const math::Vec2& min, const math::Vec2& max,
                              const Color& border_color, float thickness) {
    boxes_.push_back({min, max, Color(0, 0, 0, 0), border_color, thickness, false});
}

void SceneVisualizer::add_filled_box(const math::Vec2& min, const math::Vec2& max,
                                     const Color& fill_color, const Color& border_color,
                                     float thickness) {
    boxes_.push_back({min, max, fill_color, border_color, thickness, true});
}

void SceneVisualizer::add_circle(const math::Vec2& center, float radius,
                                 const Color& color, float thickness) {
    circles_.push_back({center, radius, color, thickness, false, 16});
}

void SceneVisualizer::add_text(const math::Vec2& pos, const char* text,
                               const Color& color, float scale, bool centered) {
    texts_.push_back({pos, text, color, scale, centered});
}

void SceneVisualizer::render_entity_2d(const core::EntityInfo& entity) {
    // Project bounding box corners
    math::BoundingBox bbox(entity.bbox_min, entity.bbox_max);
    std::array<math::Vec2, 8> corners;
    math::Vec2 min_screen, max_screen;
    
    if (!bbox.project_to_screen(current_view_, corners, min_screen, max_screen)) {
        return;
    }
    
    // Calculate 2D box with padding
    const float padding = 2.0f;
    math::Vec2 box_min(min_screen.x - padding, min_screen.y - padding);
    math::Vec2 box_max(max_screen.x + padding, max_screen.y + padding);
    
    // Determine color based on team/health
    Color box_color;
    if (entity.is_local_player) {
        box_color = Color(0.0f, 1.0f, 1.0f, 0.9f);  // Cyan
    } else {
        float health_ratio = static_cast<float>(entity.health) / entity.max_health;
        box_color = Color::from_health(health_ratio);
    }
    
    // Apply distance fade
    float distance = current_view_.camera_position.distance(entity.world_position);
    box_color = box_color.with_distance_fade(distance, config_.max_render_distance);
    
    // Draw box
    add_box(box_min, box_max, box_color, config_.box_thickness);
    
    // Draw line from screen center if enabled
    if (config_.show_lines && !entity.is_local_player) {
        math::Vec2 screen_center(
            current_view_.screen_width * 0.5f,
            current_view_.screen_height * 0.5f
        );
        
        // Only draw line if target is on screen
        if (entity.screen_position.x > 0 && entity.screen_position.y > 0) {
            Color line_color = box_color;
            line_color.a *= 0.5f;
            add_line(screen_center, entity.screen_position, line_color, config_.line_thickness);
        }
    }
}

void SceneVisualizer::render_entity_3d(const core::EntityInfo& entity) {
    // 3D box rendering requires all 8 corners
    math::BoundingBox bbox(entity.bbox_min, entity.bbox_max);
    std::array<math::Vec2, 8> corners_2d;
    std::array<math::Vec3, 8> corners_3d = bbox.get_corners();
    
    bool any_visible = false;
    for (int i = 0; i < 8; ++i) {
        if (current_view_.world_to_screen(corners_3d[i], corners_2d[i])) {
            any_visible = true;
        }
    }
    
    if (!any_visible) return;
    
    // Box edges (indices into corners array)
    constexpr int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},  // Bottom face
        {4, 5}, {5, 6}, {6, 7}, {7, 4},  // Top face
        {0, 4}, {1, 5}, {2, 6}, {3, 7}   // Vertical edges
    };
    
    Color edge_color = Color::from_team(entity.team_id, entity.is_local_player);
    float distance = current_view_.camera_position.distance(entity.world_position);
    edge_color = edge_color.with_distance_fade(distance, config_.max_render_distance);
    
    for (const auto& edge : edges) {
        // Only draw if both corners project to screen
        if (corners_2d[edge[0]].x != 0 || corners_2d[edge[0]].y != 0) {
            if (corners_2d[edge[1]].x != 0 || corners_2d[edge[1]].y != 0) {
                add_line(corners_2d[edge[0]], corners_2d[edge[1]], 
                        edge_color, config_.box_thickness);
            }
        }
    }
}

void SceneVisualizer::render_skeleton(const core::EntityInfo& entity) {
    Color bone_color = entity.is_local_player 
        ? Color(0.0f, 1.0f, 1.0f, 0.8f)
        : Color::from_team(entity.team_id, false);
    
    float distance = current_view_.camera_position.distance(entity.world_position);
    bone_color = bone_color.with_distance_fade(distance, config_.max_render_distance);
    
    // Draw each bone connection
    for (size_t i = 0; i < VisualConfig::skeleton_bone_count; ++i) {
        auto [parent_idx, child_idx] = VisualConfig::skeleton_bones[i];
        
        size_t p_idx = static_cast<size_t>(parent_idx);
        size_t c_idx = static_cast<size_t>(child_idx);
        
        if (p_idx < entity.bones.size() && c_idx < entity.bones.size()) {
            if (entity.bones[p_idx].valid && entity.bones[c_idx].valid) {
                math::Vec2 parent_screen, child_screen;
                
                bool parent_visible = current_view_.world_to_screen(
                    entity.bones[p_idx].position, parent_screen
                );
                bool child_visible = current_view_.world_to_screen(
                    entity.bones[c_idx].position, child_screen
                );
                
                if (parent_visible && child_visible) {
                    // Vary thickness based on bone importance
                    float thickness = config_.skeleton_thickness;
                    if (parent_idx == core::BoneIndex::Head) {
                        thickness *= 1.5f;  // Thicker for head connection
                    }
                    
                    add_line(parent_screen, child_screen, bone_color, thickness);
                }
            }
        }
    }
    
    // Draw head circle
    const auto& head_bone = entity.bones[static_cast<size_t>(core::BoneIndex::Head)];
    if (head_bone.valid) {
        math::Vec2 head_screen;
        if (current_view_.world_to_screen(head_bone.position, head_screen)) {
            // Estimate head radius based on distance
            float head_radius = 10.0f * (100.0f / distance);
            add_circle(head_screen, head_radius, bone_color, 1.0f);
        }
    }
}

void SceneVisualizer::render_health_bar(const core::EntityInfo& entity, 
                                        const math::Vec2& screen_pos) {
    if (entity.max_health <= 0) return;
    
    // Position bar above entity
    const float bar_width = 40.0f;
    const float bar_height = 4.0f;
    const float vertical_offset = -50.0f;
    
    math::Vec2 bar_center(screen_pos.x, screen_pos.y + vertical_offset);
    
    float health_ratio = static_cast<float>(entity.health) / entity.max_health;
    health_ratio = std::clamp(health_ratio, 0.0f, 1.0f);
    
    // Background (dark)
    math::Vec2 bg_min(bar_center.x - bar_width * 0.5f, bar_center.y - bar_height * 0.5f);
    math::Vec2 bg_max(bar_center.x + bar_width * 0.5f, bar_center.y + bar_height * 0.5f);
    add_filled_box(bg_min, bg_max, Color(0.1f, 0.1f, 0.1f, 0.8f), Color(0, 0, 0, 0), 0);
    
    // Health fill
    float fill_width = bar_width * health_ratio;
    Color health_color = Color::from_health(health_ratio);
    
    math::Vec2 fill_min(bg_min.x, bg_min.y);
    math::Vec2 fill_max(bg_min.x + fill_width, bg_max.y);
    add_filled_box(fill_min, fill_max, health_color, Color(0, 0, 0, 0), 0);
    
    // Border
    add_box(bg_min, bg_max, Color(0.3f, 0.3f, 0.3f, 0.9f), 1.0f);
}

void SceneVisualizer::render_trajectory(const core::EntityInfo& entity) {
    // Predict future positions
    constexpr int prediction_steps = 10;
    constexpr float step_time = 0.05f;  // 50ms steps
    
    std::array<math::Vec3, prediction_steps> predicted_positions;
    std::array<math::Vec2, prediction_steps> screen_positions;
    
    for (int i = 0; i < prediction_steps; ++i) {
        predicted_positions[i] = entity.predict_position(step_time * (i + 1));
    }
    
    // Project to screen
    int valid_points = 0;
    for (int i = 0; i < prediction_steps; ++i) {
        if (current_view_.world_to_screen(predicted_positions[i], screen_positions[i])) {
            valid_points++;
        }
    }
    
    if (valid_points < 2) return;
    
    // Draw trajectory line
    Color traj_color = Color(1.0f, 0.5f, 0.0f, 0.6f);  // Orange
    
    for (int i = 0; i < valid_points - 1; ++i) {
        float alpha = 1.0f - (static_cast<float>(i) / prediction_steps);
        Color point_color = traj_color;
        point_color.a *= alpha;
        
        add_line(screen_positions[i], screen_positions[i + 1], point_color, 1.0f);
    }
}

bool SceneVisualizer::is_entity_visible(const core::EntityInfo& entity) const {
    // Basic visibility check - more sophisticated occlusion testing would go here
    if (entity.state != core::EntityState::Alive) return false;
    
    float distance = current_view_.camera_position.distance(entity.world_position);
    if (distance > config_.max_render_distance) return false;
    
    return current_view_.in_fov(entity.world_position, 120.0f);
}

} // namespace diag::features

#pragma once

#include "../math/vector.hpp"
#include "../core/entity_manager.hpp"
#include <vector>
#include <cstdint>

namespace diag::features {

// Visual element types
enum class VisualType : uint8_t {
    BoundingBox2D,      // 2D box around entity
    BoundingBox3D,      // 3D oriented box
    Skeleton,           // Bone connections
    HealthBar,          // Health indicator
    NameLabel,          // Entity name/tag
    LineToTarget,       // Line from screen center
    Trajectory,         // Velocity prediction path
    Hitbox              // Hit area visualization
};

// Color with alpha
struct Color {
    float r, g, b, a;
    
    constexpr Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    constexpr Color(float r, float g, float b, float a = 1.0f) 
        : r(r), g(g), b(b), a(a) {}
    
    // Health-based color gradient (green -> yellow -> red)
    [[nodiscard]] static Color from_health(float health_ratio) {
        if (health_ratio > 0.5f) {
            float t = (health_ratio - 0.5f) * 2.0f;
            return Color(t, 1.0f, 0.0f, 0.9f);  // Yellow to green
        } else {
            float t = health_ratio * 2.0f;
            return Color(1.0f, t, 0.0f, 0.9f);  // Red to yellow
        }
    }
    
    // Team-based color
    [[nodiscard]] static Color from_team(int team_id, bool is_local) {
        if (is_local) return Color(0.0f, 1.0f, 1.0f, 0.9f);  // Cyan for self
        if (team_id == 0) return Color(1.0f, 0.0f, 0.0f, 0.9f);  // Red
        if (team_id == 1) return Color(0.0f, 0.0f, 1.0f, 0.9f);  // Blue
        return Color(1.0f, 1.0f, 0.0f, 0.9f);  // Yellow default
    }
    
    // Distance-based fade
    [[nodiscard]] Color with_distance_fade(float distance, float max_distance) const {
        float alpha = 1.0f - std::min(distance / max_distance, 1.0f);
        return Color(r, g, b, a * alpha * 0.7f + 0.3f);
    }
    
    [[nodiscard]] uint32_t to_u32() const {
        return (static_cast<uint32_t>(a * 255.0f) << 24) |
               (static_cast<uint32_t>(b * 255.0f) << 16) |
               (static_cast<uint32_t>(g * 255.0f) << 8) |
               (static_cast<uint32_t>(r * 255.0f));
    }
};

// Render primitive for batching
struct LinePrimitive {
    math::Vec2 start;
    math::Vec2 end;
    Color color;
    float thickness;
};

struct BoxPrimitive {
    math::Vec2 min;
    math::Vec2 max;
    Color fill_color;
    Color border_color;
    float border_thickness;
    bool filled;
};

struct TextPrimitive {
    math::Vec2 position;
    const char* text;
    Color color;
    float scale;
    bool centered;
};

struct CirclePrimitive {
    math::Vec2 center;
    float radius;
    Color color;
    float thickness;
    bool filled;
    int segments;
};

// Configuration for visualization
struct VisualConfig {
    bool enabled = true;
    
    // Toggle individual elements
    bool show_2d_boxes = true;
    bool show_3d_boxes = false;  // More expensive
    bool show_skeleton = true;
    bool show_health = true;
    bool show_names = false;
    bool show_lines = true;
    bool show_trajectory = false;
    
    // Distance culling
    float max_render_distance = 500.0f;
    
    // Visual parameters
    float box_thickness = 1.5f;
    float skeleton_thickness = 1.0f;
    float line_thickness = 1.0f;
    
    // Skeleton connections (bone pairs)
    // Format: {parent_bone, child_bone}
    static constexpr std::pair<core::BoneIndex, core::BoneIndex> skeleton_bones[] = {
        {core::BoneIndex::Head, core::BoneIndex::Neck},
        {core::BoneIndex::Neck, core::BoneIndex::Spine2},
        {core::BoneIndex::Spine2, core::BoneIndex::Spine1},
        {core::BoneIndex::Spine1, core::BoneIndex::Pelvis},
        {core::BoneIndex::Neck, core::BoneIndex::LeftShoulder},
        {core::BoneIndex::LeftShoulder, core::BoneIndex::LeftArm},
        {core::BoneIndex::LeftArm, core::BoneIndex::LeftForearm},
        {core::BoneIndex::LeftForearm, core::BoneIndex::LeftHand},
        {core::BoneIndex::Neck, core::BoneIndex::RightShoulder},
        {core::BoneIndex::RightShoulder, core::BoneIndex::RightArm},
        {core::BoneIndex::RightArm, core::BoneIndex::RightForearm},
        {core::BoneIndex::RightForearm, core::BoneIndex::RightHand},
        {core::BoneIndex::Pelvis, core::BoneIndex::LeftThigh},
        {core::BoneIndex::LeftThigh, core::BoneIndex::LeftCalf},
        {core::BoneIndex::LeftCalf, core::BoneIndex::LeftFoot},
        {core::BoneIndex::Pelvis, core::BoneIndex::RightThigh},
        {core::BoneIndex::RightThigh, core::BoneIndex::RightCalf},
        {core::BoneIndex::RightCalf, core::BoneIndex::RightFoot}
    };
    static constexpr size_t skeleton_bone_count = sizeof(skeleton_bones) / sizeof(skeleton_bones[0]);
};

// Scene visualizer for entity overlay rendering
class SceneVisualizer {
public:
    static SceneVisualizer& instance() {
        static SceneVisualizer inst;
        return inst;
    }
    
    void initialize(const VisualConfig& config);
    
    // Update with current view
    void update_view(const math::ViewData& view) { current_view_ = view; }
    
    // Main render function - builds primitive lists
    void prepare_frame(const std::vector<core::EntityInfo>& entities);
    
    // Draw all primitives (called from render hook)
    void render_frame();
    
    // Access primitives for custom rendering
    [[nodiscard]] const std::vector<LinePrimitive>& get_lines() const { return lines_; }
    [[nodiscard]] const std::vector<BoxPrimitive>& get_boxes() const { return boxes_; }
    [[nodiscard]] const std::vector<CirclePrimitive>& get_circles() const { return circles_; }
    [[nodiscard]] const std::vector<TextPrimitive>& get_texts() const { return texts_; }
    
    // Clear primitives
    void clear_primitives();
    
    // Add individual primitives
    void add_line(const math::Vec2& start, const math::Vec2& end, 
                  const Color& color, float thickness = 1.0f);
    void add_box(const math::Vec2& min, const math::Vec2& max,
                 const Color& border_color, float thickness = 1.0f);
    void add_filled_box(const math::Vec2& min, const math::Vec2& max,
                        const Color& fill_color, const Color& border_color,
                        float thickness = 1.0f);
    void add_circle(const math::Vec2& center, float radius, 
                    const Color& color, float thickness = 1.0f);
    void add_text(const math::Vec2& pos, const char* text, 
                  const Color& color, float scale = 1.0f, bool centered = false);
    
    // Config access
    void set_config(const VisualConfig& config) { config_ = config; }
    [[nodiscard]] const VisualConfig& get_config() const { return config_; }
    
    // Statistics
    struct RenderStats {
        int entities_rendered;
        int primitives_drawn;
        float avg_time_ms;
    };
    [[nodiscard]] RenderStats get_stats() const { return stats_; }

private:
    SceneVisualizer() = default;
    
    void render_entity_2d(const core::EntityInfo& entity);
    void render_entity_3d(const core::EntityInfo& entity);
    void render_skeleton(const core::EntityInfo& entity);
    void render_health_bar(const core::EntityInfo& entity, const math::Vec2& screen_pos);
    void render_trajectory(const core::EntityInfo& entity);
    
    [[nodiscard]] bool is_entity_visible(const core::EntityInfo& entity) const;
    
    VisualConfig config_;
    math::ViewData current_view_;
    
    // Primitive buffers
    std::vector<LinePrimitive> lines_;
    std::vector<BoxPrimitive> boxes_;
    std::vector<CirclePrimitive> circles_;
    std::vector<TextPrimitive> texts_;
    
    RenderStats stats_ = {};
};

// Bone-to-bone connection for skeleton rendering
struct BoneConnection {
    core::BoneIndex parent;
    core::BoneIndex child;
    Color color;
};

} // namespace diag::features

#pragma once

#include "memory.hpp"
#include "../math/vector.hpp"
#include <vector>
#include <array>
#include <cstdint>

namespace diag::core {

// Bone indices for skeletal hierarchy
enum class BoneIndex : uint32_t {
    Root = 0,
    Pelvis = 1,
    Spine = 2,
    Spine1 = 3,
    Spine2 = 4,
    Neck = 5,
    Head = 6,
    LeftShoulder = 7,
    LeftArm = 8,
    LeftForearm = 9,
    LeftHand = 10,
    RightShoulder = 11,
    RightArm = 12,
    RightForearm = 13,
    RightHand = 14,
    LeftThigh = 15,
    LeftCalf = 16,
    LeftFoot = 17,
    RightThigh = 18,
    RightCalf = 19,
    RightFoot = 20,
    MaxBones
};

// Priority for aim target selection
[[nodiscard]] constexpr int get_bone_priority(BoneIndex bone) {
    switch (bone) {
        case BoneIndex::Head: return 100;
        case BoneIndex::Neck: return 90;
        case BoneIndex::Spine2: return 80;  // Upper chest
        case BoneIndex::Spine1: return 70;  // Chest
        case BoneIndex::Pelvis: return 60;
        case BoneIndex::LeftShoulder:
        case BoneIndex::RightShoulder: return 50;
        default: return 10;
    }
}

// Entity state for visualization
enum class EntityState : uint8_t {
    Alive = 0,
    Dead = 1,
    Dormant = 2,
    Spectating = 3
};

// Transform data for a single bone
struct BoneData {
    math::Vec3 position;
    math::Quaternion rotation;
    float scale;
    bool valid;
};

// Cached entity information
struct EntityInfo {
    uintptr_t base_address;           // IL2CPP object base
    uint32_t entity_id;
    EntityState state;
    
    // Health and team data
    int health;
    int max_health;
    int team_id;
    bool is_local_player;
    
    // Transform data
    math::Vec3 world_position;
    math::Vec3 velocity;
    std::array<BoneData, static_cast<size_t>(BoneIndex::MaxBones)> bones;
    
    // Cached 2D projection
    math::Vec2 screen_position;
    float screen_distance;  // Distance from center of screen
    bool on_screen;
    
    // Bounding box (calculated from bone positions)
    math::Vec3 bbox_min;
    math::Vec3 bbox_max;
    
    // Velocity prediction for latency compensation
    [[nodiscard]] math::Vec3 predict_position(float delta_time) const {
        return world_position + velocity * delta_time;
    }
    
    // Get bone position with validation
    [[nodiscard]] math::Vec3 get_bone_position(BoneIndex bone) const {
        size_t idx = static_cast<size_t>(bone);
        if (idx < bones.size() && bones[idx].valid) {
            return bones[idx].position;
        }
        return world_position;
    }
    
    // Calculate center of mass (pelvis/spine area)
    [[nodiscard]] math::Vec3 get_center_mass() const {
        return get_bone_position(BoneIndex::Pelvis);
    }
    
    // Calculate visibility score (higher = more visible)
    [[nodiscard]] float get_visibility_score() const {
        if (state != EntityState::Alive) return 0.0f;
        float health_ratio = static_cast<float>(health) / max_health;
        float bone_validity = 0.0f;
        int valid_count = 0;
        
        for (const auto& bone : bones) {
            if (bone.valid) valid_count++;
        }
        bone_validity = static_cast<float>(valid_count) / bones.size();
        
        return health_ratio * bone_validity;
    }
};

// Configuration for entity scanning
struct EntityScanConfig {
    uintptr_t entity_list_offset;      // Offset to entity list in game structure
    uintptr_t entity_count_offset;     // Offset to entity count
    uintptr_t entity_size;             // Size of each entity entry
    
    // Field offsets within entity structure (relative to IL2CppObject)
    uint32_t health_offset;
    uint32_t max_health_offset;
    uint32_t team_offset;
    uint32_t position_offset;
    uint32_t velocity_offset;
    uint32_t bone_matrix_offset;
    uint32_t state_offset;
    
    // Validation
    uint32_t entity_signature;         // Expected value for valid entity
};

// Manager for entity iteration and caching
class EntityManager {
public:
    static EntityManager& instance() {
        static EntityManager inst;
        return inst;
    }
    
    bool initialize(const EntityScanConfig& config, uintptr_t module_base);
    
    // Update entity cache - call once per frame
    void update_cache();
    
    // Access cached entities
    [[nodiscard]] const std::vector<EntityInfo>& get_entities() const {
        return cached_entities_;
    }
    
    // Get local player
    [[nodiscard]] const EntityInfo* get_local_player() const;
    
    // Find best target based on criteria
    [[nodiscard]] const EntityInfo* find_best_target(
        const math::Vec3& source_pos,
        float max_distance,
        float max_fov,
        BoneIndex preferred_bone
    ) const;
    
    // Get entity by ID
    [[nodiscard]] const EntityInfo* get_entity(uint32_t id) const;
    
    // Calculate bone transform matrices
    void update_bone_transforms(EntityInfo& entity);
    
    // Config accessors
    void set_config(const EntityScanConfig& config) { config_ = config; }
    [[nodiscard]] const EntityScanConfig& get_config() const { return config_; }

private:
    EntityManager() = default;
    
    bool read_entity_at_index(size_t index, EntityInfo& out);
    void validate_entity(EntityInfo& entity);
    
    EntityScanConfig config_;
    uintptr_t module_base_ = 0;
    uintptr_t entity_list_ptr_ = 0;
    
    std::vector<EntityInfo> cached_entities_;
    EntityInfo* local_player_ = nullptr;
    
    // Performance: use fixed buffer for small entity counts
    static constexpr size_t MAX_CACHED_ENTITIES = 64;
};

// Ray/OBB intersection for occlusion testing (simplified)
[[nodiscard]] bool intersect_ray_obb(
    const math::Vec3& ray_origin,
    const math::Vec3& ray_dir,
    const math::Vec3& box_center,
    const math::Vec3& box_extents,
    const math::Vec3& box_axes[3]
);

} // namespace diag::core

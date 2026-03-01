#include "entity_manager.hpp"
#include "memory.hpp"
#include <algorithm>
#include <cmath>

namespace diag::core {

bool EntityManager::initialize(const EntityScanConfig& config, uintptr_t module_base) {
    config_ = config;
    module_base_ = module_base;
    
    // Resolve entity list pointer from module base
    ProcessMemory& mem = ProcessMemory::instance();
    
    if (!mem.read(module_base_ + config_.entity_list_offset, entity_list_ptr_)) {
        return false;
    }
    
    cached_entities_.reserve(MAX_CACHED_ENTITIES);
    return entity_list_ptr_ != 0;
}

void EntityManager::update_cache() {
    ProcessMemory& mem = ProcessMemory::instance();
    cached_entities_.clear();
    local_player_ = nullptr;
    
    // Read entity count
    int entity_count = 0;
    if (!mem.read(entity_list_ptr_ + config_.entity_count_offset, entity_count)) {
        return;
    }
    
    // Clamp to reasonable maximum
    entity_count = std::min(entity_count, static_cast<int>(MAX_CACHED_ENTITIES));
    
    for (int i = 0; i < entity_count; ++i) {
        EntityInfo entity;
        if (read_entity_at_index(i, entity)) {
            validate_entity(entity);
            
            if (entity.state == EntityState::Alive) {
                cached_entities_.push_back(entity);
                
                if (entity.is_local_player) {
                    local_player_ = &cached_entities_.back();
                }
            }
        }
    }
}

bool EntityManager::read_entity_at_index(size_t index, EntityInfo& out) {
    ProcessMemory& mem = ProcessMemory::instance();
    
    // Calculate entity pointer
    uintptr_t entity_ptr;
    uintptr_t entry_addr = entity_list_ptr_ + index * sizeof(uintptr_t);
    
    if (!mem.read(entry_addr, entity_ptr) || entity_ptr == 0) {
        return false;
    }
    
    // Read IL2CppObject header
    Il2CppObject obj;
    if (!mem.read(entity_ptr, obj)) {
        return false;
    }
    
    // Validate vtable
    if (obj.vtable == nullptr) {
        return false;
    }
    
    out.base_address = entity_ptr;
    out.entity_id = static_cast<uint32_t>(index);
    
    // Read health
    Il2CppField<int> health_field(config_.health_offset);
    mem.read(health_field.get_ptr(entity_ptr), out.health);
    
    Il2CppField<int> max_health_field(config_.max_health_offset);
    mem.read(max_health_field.get_ptr(entity_ptr), out.max_health);
    
    // Read team
    Il2CppField<int> team_field(config_.team_offset);
    mem.read(team_field.get_ptr(entity_ptr), out.team_id);
    
    // Read position
    Il2CppField<math::Vec3> pos_field(config_.position_offset);
    mem.read(pos_field.get_ptr(entity_ptr), out.world_position);
    
    // Read velocity
    Il2CppField<math::Vec3> vel_field(config_.velocity_offset);
    mem.read(vel_field.get_ptr(entity_ptr), out.velocity);
    
    // Read state
    Il2CppField<uint8_t> state_field(config_.state_offset);
    uint8_t state_val;
    if (mem.read(state_field.get_ptr(entity_ptr), state_val)) {
        out.state = static_cast<EntityState>(state_val);
    } else {
        out.state = EntityState::Dormant;
    }
    
    // Read bone matrix if available
    if (config_.bone_matrix_offset != 0) {
        update_bone_transforms(out);
    }
    
    return true;
}

void EntityManager::validate_entity(EntityInfo& entity) {
    // Health validation
    if (entity.health < 0) entity.health = 0;
    if (entity.health > 1000) entity.health = 1000;  // Sanity check
    if (entity.max_health <= 0) entity.max_health = 100;
    
    // Position validation
    const float MAX_COORD = 10000.0f;
    if (std::abs(entity.world_position.x) > MAX_COORD ||
        std::abs(entity.world_position.y) > MAX_COORD ||
        std::abs(entity.world_position.z) > MAX_COORD) {
        entity.state = EntityState::Dormant;
    }
    
    // Calculate bounding box from bone positions or use defaults
    if (entity.bones[static_cast<size_t>(BoneIndex::Head)].valid &&
        entity.bones[static_cast<size_t>(BoneIndex::LeftFoot)].valid) {
        
        entity.bbox_min = entity.world_position;
        entity.bbox_max = entity.world_position;
        
        for (const auto& bone : entity.bones) {
            if (bone.valid) {
                entity.bbox_min.x = std::min(entity.bbox_min.x, bone.position.x);
                entity.bbox_min.y = std::min(entity.bbox_min.y, bone.position.y);
                entity.bbox_min.z = std::min(entity.bbox_min.z, bone.position.z);
                entity.bbox_max.x = std::max(entity.bbox_max.x, bone.position.x);
                entity.bbox_max.y = std::max(entity.bbox_max.y, bone.position.y);
                entity.bbox_max.z = std::max(entity.bbox_max.z, bone.position.z);
            }
        }
    } else {
        // Default humanoid bounds
        entity.bbox_min = entity.world_position - math::Vec3(30.0f, 0.0f, 30.0f);
        entity.bbox_max = entity.world_position + math::Vec3(30.0f, 180.0f, 30.0f);
    }
}

void EntityManager::update_bone_transforms(EntityInfo& entity) {
    ProcessMemory& mem = ProcessMemory::instance();
    
    // Bone matrix is typically an array of transform matrices
    uintptr_t bone_matrix_addr;
    Il2CppField<uintptr_t> bone_field(config_.bone_matrix_offset);
    
    if (!mem.read(bone_field.get_ptr(entity.base_address), bone_matrix_addr)) {
        return;
    }
    
    // Read bone positions from matrix
    // Standard Unity humanoid has ~20-24 bones
    for (size_t i = 0; i < static_cast<size_t>(BoneIndex::MaxBones) && i < 24; ++i) {
        // Bone matrix entries are typically 4x4 matrices (64 bytes each)
        math::Vec3 bone_pos;
        if (mem.read(bone_matrix_addr + i * 64 + 48, bone_pos)) {  // Translation component
            entity.bones[i].position = bone_pos;
            entity.bones[i].valid = true;
        } else {
            entity.bones[i].valid = false;
        }
    }
}

const EntityInfo* EntityManager::get_local_player() const {
    return local_player_;
}

const EntityInfo* EntityManager::find_best_target(
    const math::Vec3& source_pos,
    float max_distance,
    float max_fov_deg,
    BoneIndex preferred_bone
) const {
    const EntityInfo* best_target = nullptr;
    float best_score = -1.0f;
    
    const float max_fov_rad = max_fov_deg * 3.14159265f / 180.0f;
    
    for (const auto& entity : cached_entities_) {
        if (entity.state != EntityState::Alive) continue;
        if (entity.is_local_player) continue;
        
        // Team check (skip teammates)
        if (local_player_ && entity.team_id == local_player_->team_id) continue;
        
        // Distance check
        float distance = source_pos.distance(entity.world_position);
        if (distance > max_distance) continue;
        
        // Get target position (use preferred bone)
        math::Vec3 target_pos = entity.get_bone_position(preferred_bone);
        
        // Calculate direction and FOV
        math::Vec3 dir = (target_pos - source_pos).normalized();
        
        // For FOV calculation, we'd need camera forward from ViewData
        // This is simplified - actual implementation would use view matrix
        
        // Scoring: prefer closer, higher health ratio, central position
        float distance_score = 1.0f - (distance / max_distance);
        float health_score = static_cast<float>(entity.health) / entity.max_health;
        float bone_priority = static_cast<float>(get_bone_priority(preferred_bone)) / 100.0f;
        
        // Combined score with weights
        float score = distance_score * 0.4f + health_score * 0.3f + bone_priority * 0.3f;
        
        if (score > best_score) {
            best_score = score;
            best_target = &entity;
        }
    }
    
    return best_target;
}

const EntityInfo* EntityManager::get_entity(uint32_t id) const {
    for (const auto& entity : cached_entities_) {
        if (entity.entity_id == id) {
            return &entity;
        }
    }
    return nullptr;
}

bool intersect_ray_obb(
    const math::Vec3& ray_origin,
    const math::Vec3& ray_dir,
    const math::Vec3& box_center,
    const math::Vec3& box_extents,
    const math::Vec3* box_axes
) {
    float t_min = 0.0f;
    float t_max = 1e30f;
    
    math::Vec3 p = box_center - ray_origin;
    
    for (int i = 0; i < 3; ++i) {
        float e = box_axes[i].dot(p);
        float f = box_axes[i].dot(ray_dir);
        
        if (std::abs(f) > 1e-6f) {
            float t1 = (e + box_extents[i]) / f;
            float t2 = (e - box_extents[i]) / f;
            
            if (t1 > t2) std::swap(t1, t2);
            
            t_min = std::max(t_min, t1);
            t_max = std::min(t_max, t2);
            
            if (t_min > t_max) return false;
        } else if (-e - box_extents[i] > 0.0f || -e + box_extents[i] < 0.0f) {
            return false;
        }
    }
    
    return true;
}

} // namespace diag::core

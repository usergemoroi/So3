#pragma once

#include "secure_types.hpp"
#include "math_ops.hpp"
#include <cstdint>
#include <atomic>

namespace dm {

// Module-relative offsets - position independent
// These would be populated from external analysis
struct game_offsets_t {
    // libil2cpp.so offsets
    static constexpr ptrdiff_t GOM_OFFSET = 0x0;      // Global Object Manager
    static constexpr ptrdiff_t BCM_OFFSET = 0x0;      // Base Class Manager
    
    // Entity offsets (relative to entity base)
    static constexpr ptrdiff_t POSITION_OFFSET = 0x0;
    static constexpr ptrdiff_t ROTATION_OFFSET = 0x0;
    static constexpr ptrdiff_t HEALTH_OFFSET = 0x0;
    static constexpr ptrdiff_t TEAM_OFFSET = 0x0;
    static constexpr ptrdiff_t BONE_MATRIX_OFFSET = 0x0;
    static constexpr ptrdiff_t VELOCITY_OFFSET = 0x0;
    
    // Camera offsets
    static constexpr ptrdiff_t CAMERA_MATRIX_OFFSET = 0x0;
    static constexpr ptrdiff_t CAMERA_POS_OFFSET = 0x0;
    static constexpr ptrdiff_t CAMERA_FOV_OFFSET = 0x0;
    
    // Bone indices for hitbox hierarchy
    enum bone_idx : int32_t {
        BONE_HEAD = 0,
        BONE_NECK = 1,
        BONE_CHEST = 2,
        BONE_SPINE = 3,
        BONE_PELVIS = 4,
        BONE_L_HAND = 5,
        BONE_R_HAND = 6,
        BONE_L_FOOT = 7,
        BONE_R_FOOT = 8,
        BONE_COUNT = 9
    };
    
    // Priority multiplier for bone selection
    [[nodiscard]] static constexpr float bone_priority(bone_idx idx) {
        switch (idx) {
            case BONE_HEAD: return 3.0f;
            case BONE_CHEST: return 2.5f;
            case BONE_PELVIS: return 2.0f;
            case BONE_NECK: return 2.2f;
            default: return 1.0f;
        }
    }
};

// Secure memory reader with validation
class safe_memory {
public:
    // Validate pointer is in mmap'ed module region
    [[nodiscard]] static bool validate(uintptr_t addr, uintptr_t module_base, size_t module_size) {
        return addr >= module_base && addr < module_base + module_size;
    }
    
    // Safe read with pointer validation
    template<typename T>
    [[nodiscard]] static T read(uintptr_t addr, uintptr_t module_base, size_t module_size) {
        if (!validate(addr, module_base, module_size)) {
            return T{};
        }
        return *reinterpret_cast<T*>(addr);
    }
    
    // Chain of pointer dereferences with validation at each step
    [[nodiscard]] static uintptr_t read_chain(uintptr_t base, const ptrdiff_t* offsets, 
                                               size_t count, uintptr_t module_base, 
                                               size_t module_size) {
        uintptr_t current = base;
        for (size_t i = 0; i < count; ++i) {
            if (!validate(current, module_base, module_size)) {
                return 0;
            }
            current = *reinterpret_cast<uintptr_t*>(current);
            if (current == 0) return 0;
            current += offsets[i];
        }
        return current;
    }
};

// Entity data cache (hot path optimization)
struct entity_cache_t {
    math::vec3_t position;
    math::vec3_t velocity;
    math::vec3_t bone_positions[game_offsets_t::BONE_COUNT];
    float health;
    int32_t team;
    bool valid;
    uint32_t last_update;
    
    void invalidate() {
        valid = false;
        health = 0;
    }
};

// Render context state
struct render_context_t {
    uintptr_t module_base;
    size_t module_size;
    
    // Cached pointers (updated each frame)
    std::atomic<uintptr_t> camera_matrix_ptr{0};
    std::atomic<uintptr_t> local_player_ptr{0};
    std::atomic<uintptr_t> entity_list_ptr{0};
    
    // Screen dimensions
    int32_t screen_width;
    int32_t screen_height;
    
    // View parameters
    float fov;
    math::vec3_t view_origin;
    math::mat4x4_t view_matrix;
    
    void update_from_game();
};

} // namespace dm

#ifndef AIMBOT_H
#define AIMBOT_H

#include "il2cpp_resolver.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

struct ModConfig;

namespace Aimbot {

struct Vector2 {
    float x, y;
    Vector2() : x(0), y(0) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}
    
    float Length() const {
        return sqrtf(x * x + y * y);
    }
};

IL2CPP::Vector3 GetBonePosition(PlayerController* player, int boneIndex) {
    if (!player) return IL2CPP::Vector3();
    
    return IL2CPP::Vector3();
}

bool IsVisible(const IL2CPP::Vector3& from, const IL2CPP::Vector3& to) {
    return true;
}

bool IsEnemy(PlayerController* player1, PlayerController* player2) {
    return true;
}

float GetFOVDistance(const IL2CPP::Vector3& targetPos, const IL2CPP::Vector3& cameraPos, const IL2CPP::Vector3& cameraForward) {
    IL2CPP::Vector3 direction = targetPos - cameraPos;
    float dx = direction.x;
    float dy = direction.y;
    float dz = direction.z;
    float length = sqrtf(dx * dx + dy * dy + dz * dz);
    
    if (length < 0.001f) return 180.0f;
    
    direction.x /= length;
    direction.y /= length;
    direction.z /= length;
    
    float dot = direction.x * cameraForward.x + 
                direction.y * cameraForward.y + 
                direction.z * cameraForward.z;
    
    float angle = acosf(dot) * (180.0f / 3.14159265359f);
    return angle;
}

PlayerController* GetBestTarget(const ModConfig& config, PlayerController* localPlayer) {
    if (!localPlayer) return nullptr;
    
    auto players = IL2CPP::GetAllPlayers();
    PlayerController* bestTarget = nullptr;
    float bestFOV = config.aimbot_fov;
    
    IL2CPP::Vector3 localPos = IL2CPP::GetTransformPosition(localPlayer->transform);
    IL2CPP::Vector3 cameraForward(0, 0, 1);
    
    for (auto player : players) {
        if (!player || player == localPlayer) continue;
        
        if (config.aimbot_team_check && !IsEnemy(localPlayer, player)) {
            continue;
        }
        
        IL2CPP::Vector3 headPos = GetBonePosition(player, 0);
        
        if (config.aimbot_visible_only && !IsVisible(localPos, headPos)) {
            continue;
        }
        
        float fov = GetFOVDistance(headPos, localPos, cameraForward);
        
        if (fov < bestFOV) {
            bestFOV = fov;
            bestTarget = player;
        }
    }
    
    return bestTarget;
}

void AimAtTarget(PlayerController* target, const ModConfig& config) {
    if (!target) return;
    
    IL2CPP::Vector3 targetPos = GetBonePosition(target, 0);
}

void Update(const ModConfig& config) {
    if (!config.aimbot_enabled) return;
    
    auto players = IL2CPP::GetAllPlayers();
    if (players.empty()) return;
    
    PlayerController* localPlayer = nullptr;
    for (auto player : players) {
        localPlayer = player;
        break;
    }
    
    if (!localPlayer) return;
    
    PlayerController* target = GetBestTarget(config, localPlayer);
    if (target) {
        AimAtTarget(target, config);
    }
}

}

#endif

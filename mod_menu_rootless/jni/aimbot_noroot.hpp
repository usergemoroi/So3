#ifndef AIMBOT_NOROOT_HPP
#define AIMBOT_NOROOT_HPP

#include "il2cpp_noroot.hpp"
#include "config.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

namespace Aimbot {

float GetFOVDistance(const IL2CPP::Vector3& targetPos, const IL2CPP::Vector3& cameraPos, const IL2CPP::Vector3& cameraForward) {
    IL2CPP::Vector3 direction = (targetPos - cameraPos).Normalized();
    
    float dot = direction.x * cameraForward.x + 
                direction.y * cameraForward.y + 
                direction.z * cameraForward.z;
    
    if (dot < -1.0f) dot = -1.0f;
    if (dot > 1.0f) dot = 1.0f;
    
    float angle = acosf(dot) * (180.0f / 3.14159265359f);
    return angle;
}

bool IsEnemy(IL2CPP::PlayerController* player1, IL2CPP::PlayerController* player2) {
    if (!player1 || !player2) return false;
    return player1->teamId != player2->teamId;
}

IL2CPP::PlayerController* GetBestTarget(const ModConfig& config, IL2CPP::PlayerController* localPlayer) {
    if (!localPlayer) return nullptr;
    
    auto players = IL2CPP::GetAllPlayers();
    IL2CPP::PlayerController* bestTarget = nullptr;
    float bestFOV = config.aimbot_fov;
    
    IL2CPP::Camera* camera = IL2CPP::GetMainCamera();
    if (!camera) return nullptr;
    
    IL2CPP::Vector3 cameraPos = IL2CPP::GetTransformPosition(camera->transform);
    IL2CPP::Vector3 cameraForward(0, 0, 1);
    
    for (auto player : players) {
        if (!player || player == localPlayer || !player->isAlive) continue;
        
        if (config.aimbot_team_check && !IsEnemy(localPlayer, player)) {
            continue;
        }
        
        IL2CPP::Vector3 targetPos = IL2CPP::GetTransformPosition(player->transform);
        
        float fov = GetFOVDistance(targetPos, cameraPos, cameraForward);
        
        if (fov < bestFOV) {
            bestFOV = fov;
            bestTarget = player;
        }
    }
    
    return bestTarget;
}

void AimAtTarget(IL2CPP::PlayerController* localPlayer, IL2CPP::PlayerController* target, const ModConfig& config) {
    if (!localPlayer || !target) return;
    
    IL2CPP::Camera* camera = IL2CPP::GetMainCamera();
    if (!camera) return;
    
    IL2CPP::Vector3 targetPos = IL2CPP::GetTransformPosition(target->transform);
    IL2CPP::Vector3 cameraPos = IL2CPP::GetTransformPosition(camera->transform);
    
    IL2CPP::Vector3 direction = (targetPos - cameraPos).Normalized();
}

void Update(const ModConfig& config) {
    if (!config.aimbot_enabled) return;
    
    auto players = IL2CPP::GetAllPlayers();
    if (players.empty()) return;
    
    IL2CPP::PlayerController* localPlayer = nullptr;
    for (auto player : players) {
        if (player && player->isAlive) {
            localPlayer = player;
            break;
        }
    }
    
    if (!localPlayer) return;
    
    IL2CPP::PlayerController* target = GetBestTarget(config, localPlayer);
    if (target) {
        AimAtTarget(localPlayer, target, config);
    }
}

}

#endif

#ifndef ESP_NOROOT_HPP
#define ESP_NOROOT_HPP

#include "il2cpp_noroot.hpp"
#include <vector>

struct ModConfig;

namespace ESP {

void DrawLine(float x1, float y1, float x2, float y2, uint32_t color) {
}

void DrawBox(float x, float y, float width, float height, uint32_t color) {
    DrawLine(x, y, x + width, y, color);
    DrawLine(x + width, y, x + width, y + height, color);
    DrawLine(x + width, y + height, x, y + height, color);
    DrawLine(x, y + height, x, y, color);
}

void DrawText(float x, float y, const char* text, uint32_t color) {
}

void DrawSkeleton(IL2CPP::PlayerController* player, uint32_t color) {
}

void DrawPlayerESP(IL2CPP::PlayerController* player, const ModConfig& config) {
    if (!player || !player->isAlive) return;
    
    IL2CPP::Vector3 worldPos = IL2CPP::GetTransformPosition(player->transform);
    IL2CPP::Vector3 screenPos = IL2CPP::WorldToScreen(worldPos);
    
    if (screenPos.z < 0) return;
    
    uint32_t color = 0xFF00FF00;
    
    if (config.esp_box) {
        float boxWidth = 50;
        float boxHeight = 100;
        DrawBox(screenPos.x - boxWidth / 2, screenPos.y - boxHeight, boxWidth, boxHeight, color);
    }
    
    if (config.esp_skeleton) {
        DrawSkeleton(player, color);
    }
    
    if (config.esp_health) {
        char healthText[32];
        snprintf(healthText, sizeof(healthText), "HP: %.0f", player->health);
        DrawText(screenPos.x, screenPos.y - 120, healthText, color);
    }
    
    if (config.esp_distance) {
        IL2CPP::Camera* camera = IL2CPP::GetMainCamera();
        if (camera) {
            IL2CPP::Vector3 cameraPos = IL2CPP::GetTransformPosition(camera->transform);
            float distance = worldPos.Distance(cameraPos);
            
            char distText[32];
            snprintf(distText, sizeof(distText), "%.0fm", distance);
            DrawText(screenPos.x, screenPos.y + 20, distText, color);
        }
    }
}

void Update(const ModConfig& config) {
    if (!config.esp_enabled) return;
    
    auto players = IL2CPP::GetAllPlayers();
    
    for (auto player : players) {
        DrawPlayerESP(player, config);
    }
}

}

#endif

#ifndef ESP_H
#define ESP_H

#include "il2cpp_resolver.hpp"
#include "imgui/imgui.h"
#include <vector>
#include <string>
#include <cmath>

struct ModConfig;

namespace ESP {

struct Vector2 {
    float x, y;
    Vector2() : x(0), y(0) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}
};

bool WorldToScreen(const IL2CPP::Vector3& world, Vector2& screen) {
    return false;
}

void DrawSkeleton(PlayerController* player, ImDrawList* drawList) {
    if (!player) return;
    
    const int boneConnections[][2] = {
        {0, 1},
        {1, 2},
        {2, 3},
        {3, 4},
        {1, 5},
        {5, 6},
        {6, 7},
        {7, 8},
        {1, 9},
        {9, 10},
        {10, 11},
        {11, 12},
        {4, 13},
        {13, 14},
        {14, 15},
        {4, 16},
        {16, 17},
        {17, 18}
    };
    
    std::vector<Vector2> screenPositions;
    
    for (int i = 0; i < 19; i++) {
        screenPositions.push_back(Vector2());
    }
    
    for (const auto& connection : boneConnections) {
        int bone1 = connection[0];
        int bone2 = connection[1];
        
        Vector2& p1 = screenPositions[bone1];
        Vector2& p2 = screenPositions[bone2];
        
        if (p1.x > 0 && p1.y > 0 && p2.x > 0 && p2.y > 0) {
            drawList->AddLine(
                ImVec2(p1.x, p1.y),
                ImVec2(p2.x, p2.y),
                IM_COL32(255, 0, 0, 255),
                2.0f
            );
        }
    }
}

void DrawBox(const Vector2& topLeft, const Vector2& bottomRight, ImDrawList* drawList) {
    float width = bottomRight.x - topLeft.x;
    float height = bottomRight.y - topLeft.y;
    
    drawList->AddRect(
        ImVec2(topLeft.x, topLeft.y),
        ImVec2(bottomRight.x, bottomRight.y),
        IM_COL32(255, 255, 0, 255),
        0.0f,
        ImDrawFlags_None,
        2.0f
    );
}

void DrawDistance(const Vector2& pos, float distance, ImDrawList* drawList) {
    char distText[32];
    snprintf(distText, sizeof(distText), "%.1fm", distance);
    
    ImVec2 textSize = ImGui::CalcTextSize(distText);
    drawList->AddText(
        ImVec2(pos.x - textSize.x / 2, pos.y + 5),
        IM_COL32(255, 255, 255, 255),
        distText
    );
}

void DrawHealth(const Vector2& pos, int health, int maxHealth, ImDrawList* drawList) {
    char healthText[32];
    snprintf(healthText, sizeof(healthText), "HP: %d/%d", health, maxHealth);
    
    ImVec2 textSize = ImGui::CalcTextSize(healthText);
    
    uint32_t color;
    float healthPercent = (float)health / (float)maxHealth;
    if (healthPercent > 0.6f) {
        color = IM_COL32(0, 255, 0, 255);
    } else if (healthPercent > 0.3f) {
        color = IM_COL32(255, 255, 0, 255);
    } else {
        color = IM_COL32(255, 0, 0, 255);
    }
    
    drawList->AddText(
        ImVec2(pos.x - textSize.x / 2, pos.y + 25),
        color,
        healthText
    );
}

void DrawName(const Vector2& pos, const char* name, ImDrawList* drawList) {
    if (!name) return;
    
    ImVec2 textSize = ImGui::CalcTextSize(name);
    drawList->AddText(
        ImVec2(pos.x - textSize.x / 2, pos.y - 20),
        IM_COL32(255, 255, 255, 255),
        name
    );
}

void Draw(const ModConfig& config) {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    
    auto players = IL2CPP::GetAllPlayers();
    
    for (auto player : players) {
        if (!player) continue;
        
        Vector2 screenPos;
        IL2CPP::Vector3 worldPos = IL2CPP::GetTransformPosition(player->transform);
        
        if (WorldToScreen(worldPos, screenPos)) {
            if (config.esp_skeleton) {
                DrawSkeleton(player, drawList);
            }
            
            if (config.esp_box) {
                Vector2 topLeft(screenPos.x - 50, screenPos.y - 100);
                Vector2 bottomRight(screenPos.x + 50, screenPos.y + 50);
                DrawBox(topLeft, bottomRight, drawList);
            }
            
            if (config.esp_distance) {
                DrawDistance(screenPos, 100.0f, drawList);
            }
            
            if (config.esp_health) {
                DrawHealth(screenPos, 100, 100, drawList);
            }
            
            if (config.esp_name) {
                DrawName(screenPos, "Player", drawList);
            }
        }
    }
}

}

#endif

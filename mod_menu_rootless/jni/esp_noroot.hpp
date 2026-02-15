#ifndef ESP_NOROOT_HPP
#define ESP_NOROOT_HPP

#include "il2cpp_noroot.hpp"
#include "config.hpp"
#include <vector>
#include <GLES/gl.h>
#include <EGL/egl.h>

namespace ESP {

// OpenGL-based rendering
struct Color {
    float r, g, b, a;
    Color(uint32_t hex) {
        r = ((hex >> 16) & 0xFF) / 255.0f;
        g = ((hex >> 8) & 0xFF) / 255.0f;
        b = (hex & 0xFF) / 255.0f;
        a = ((hex >> 24) & 0xFF) / 255.0f;
    }
};

inline void SetupOrtho() {
    glPushMatrix();
    glLoadIdentity();
    glOrthof(0.0f, 1080.0f, 1920.0f, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

inline void RestoreMatrix() {
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

inline void DrawLine(float x1, float y1, float x2, float y2, const Color& col) {
    glColor4f(col.r, col.g, col.b, col.a);
    glLineWidth(2.0f);
    
    GLfloat vertices[] = {
        x1, y1,
        x2, y2
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableClientState(GL_VERTEX_ARRAY);
}

inline void DrawRect(float x, float y, float width, float height, const Color& col) {
    glColor4f(col.r, col.g, col.b, col.a);
    
    GLfloat vertices[] = {
        x, y,
        x + width, y,
        x + width, y + height,
        x, y + height
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

inline void DrawFilledRect(float x, float y, float width, float height, const Color& col) {
    glColor4f(col.r, col.g, col.b, col.a);
    
    GLfloat vertices[] = {
        x, y,
        x + width, y,
        x + width, y + height,
        x, y + height
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

inline void DrawBox(float x, float y, float width, float height, uint32_t color) {
    Color col(color);
    DrawRect(x - width/2, y - height, width, height, col);
}

inline void DrawHealthBar(float x, float y, float width, float height, float health, float maxHealth) {
    float healthPercent = health / maxHealth;
    
    // Background (red)
    DrawFilledRect(x - width/2, y - height - 10, width, 5, Color(0xFFFF0000));
    
    // Health (green)
    DrawFilledRect(x - width/2, y - height - 10, width * healthPercent, 5, Color(0xFF00FF00));
}

inline void DrawSkeleton(IL2CPP::PlayerController* player, uint32_t color) {
    // Simplified skeleton - connect key bones
    // This would need actual bone positions from game
    Color col(color);
    
    // Placeholder - draw lines between estimated positions
    IL2CPP::Vector3 pos = IL2CPP::GetTransformPosition(player->transform);
    IL2CPP::Vector3 screenPos = IL2CPP::WorldToScreen(pos);
    
    if (screenPos.z < 0) return;
    
    // Draw simple cross for now
    float size = 50.0f;
    DrawLine(screenPos.x - size, screenPos.y, screenPos.x + size, screenPos.y, col);
    DrawLine(screenPos.x, screenPos.y - size, screenPos.x, screenPos.y + size, col);
}

inline void DrawPlayerESP(IL2CPP::PlayerController* player, const ModConfig& config) {
    if (!player || !player->isAlive) return;
    
    IL2CPP::Vector3 worldPos = IL2CPP::GetTransformPosition(player->transform);
    IL2CPP::Vector3 screenPos = IL2CPP::WorldToScreen(worldPos);
    
    if (screenPos.z < 0) return;  // Behind camera
    
    uint32_t color = 0xFF00FF00;  // Green
    
    // Calculate box size based on distance
    float distance = 10.0f;  // Placeholder
    float boxWidth = 1000.0f / distance;
    float boxHeight = 2000.0f / distance;
    
    if (config.esp_box) {
        DrawBox(screenPos.x, screenPos.y, boxWidth, boxHeight, color);
    }
    
    if (config.esp_skeleton) {
        DrawSkeleton(player, color);
    }
    
    if (config.esp_health) {
        DrawHealthBar(screenPos.x, screenPos.y, boxWidth, boxHeight, player->health, player->maxHealth);
    }
}

// Hook into game's render function
void (*orig_glClear)(GLbitfield mask) = nullptr;
void hooked_glClear(GLbitfield mask) {
    orig_glClear(mask);
    
    // Render ESP after clear
    if (g_Config.esp_enabled) {
        glPushMatrix();
        glLoadIdentity();
        
        // Save current state
        GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
        GLboolean texture2D = glIsEnabled(GL_TEXTURE_2D);
        
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        
        // Setup orthographic projection for 2D drawing
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrthof(0, viewport[2], viewport[3], 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        
        // Draw ESP for all players
        auto players = IL2CPP::GetAllPlayers();
        for (auto player : players) {
            DrawPlayerESP(player, g_Config);
        }
        
        // Restore state
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        
        if (depthTest) glEnable(GL_DEPTH_TEST);
        if (texture2D) glEnable(GL_TEXTURE_2D);
        
        glPopMatrix();
    }
}

inline void Init() {
    // Hook glClear for ESP rendering
    void* libgles = dlopen("libGLESv2.so", RTLD_NOW);
    if (libgles) {
        void* glClear = dlsym(libgles, "glClear");
        if (glClear) {
            MSHookFunction(glClear, (void*)hooked_glClear, (void**)&orig_glClear);
        }
    }
}

inline void Update(const ModConfig& config) {
    // ESP is now rendered in hooked_glClear
    // This function can be used for updates
}

}
#endif

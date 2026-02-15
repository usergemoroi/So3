#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include <unistd.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include "il2cpp_resolver.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_android.h"
#include "esp.h"
#include "aimbot.h"

#define LOG_TAG "ModMenu"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct ModConfig {
    bool esp_enabled = false;
    bool esp_skeleton = false;
    bool esp_box = false;
    bool esp_distance = false;
    bool esp_health = false;
    bool esp_name = false;
    
    bool aimbot_enabled = false;
    bool aimbot_visible_only = true;
    bool aimbot_team_check = true;
    float aimbot_fov = 90.0f;
    float aimbot_smooth = 5.0f;
    
    bool menu_visible = true;
};

ModConfig g_Config;
bool g_Initialized = false;
bool g_ImGuiInitialized = false;

void (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!g_ImGuiInitialized) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui_ImplAndroid_Init(nullptr);
        
        g_ImGuiInitialized = true;
        LOGI("ImGui initialized");
    }
    
    if (g_ImGuiInitialized) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();
        
        static float menu_x = 100.0f;
        static float menu_y = 100.0f;
        static bool dragging = false;
        static ImVec2 drag_offset;
        
        if (g_Config.menu_visible) {
            ImGui::SetNextWindowPos(ImVec2(menu_x, menu_y), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
            
            ImGui::Begin("Standoff 2 Mod Menu", &g_Config.menu_visible, 
                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
            
            if (ImGui::CollapsingHeader("ESP Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Enable ESP", &g_Config.esp_enabled);
                ImGui::Separator();
                
                if (g_Config.esp_enabled) {
                    ImGui::Checkbox("Skeleton", &g_Config.esp_skeleton);
                    ImGui::Checkbox("Box", &g_Config.esp_box);
                    ImGui::Checkbox("Distance", &g_Config.esp_distance);
                    ImGui::Checkbox("Health", &g_Config.esp_health);
                    ImGui::Checkbox("Name", &g_Config.esp_name);
                }
            }
            
            if (ImGui::CollapsingHeader("Aimbot Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Enable Aimbot", &g_Config.aimbot_enabled);
                ImGui::Separator();
                
                if (g_Config.aimbot_enabled) {
                    ImGui::Checkbox("Visible Only", &g_Config.aimbot_visible_only);
                    ImGui::Checkbox("Team Check", &g_Config.aimbot_team_check);
                    ImGui::SliderFloat("FOV", &g_Config.aimbot_fov, 10.0f, 180.0f);
                    ImGui::SliderFloat("Smooth", &g_Config.aimbot_smooth, 1.0f, 20.0f);
                }
            }
            
            ImGui::Separator();
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            
            ImGui::End();
        }
        
        static float button_x = 20.0f;
        static float button_y = 20.0f;
        static bool button_dragging = false;
        static ImVec2 button_drag_offset;
        
        ImGui::SetNextWindowPos(ImVec2(button_x, button_y), ImGuiCond_FirstUseEver);
        ImGui::Begin("##MenuButton", nullptr, 
                    ImGuiWindowFlags_NoTitleBar | 
                    ImGuiWindowFlags_NoResize | 
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_AlwaysAutoResize);
        
        ImVec2 window_pos = ImGui::GetWindowPos();
        ImVec2 window_size = ImGui::GetWindowSize();
        
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(0)) {
            if (!button_dragging) {
                button_dragging = true;
                ImVec2 mouse_pos = ImGui::GetMousePos();
                button_drag_offset = ImVec2(mouse_pos.x - window_pos.x, mouse_pos.y - window_pos.y);
            }
        }
        
        if (button_dragging && ImGui::IsMouseDown(0)) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            button_x = mouse_pos.x - button_drag_offset.x;
            button_y = mouse_pos.y - button_drag_offset.y;
            ImGui::SetWindowPos(ImVec2(button_x, button_y));
        }
        
        if (!ImGui::IsMouseDown(0)) {
            button_dragging = false;
        }
        
        if (ImGui::Button(g_Config.menu_visible ? "Hide Menu" : "Show Menu", ImVec2(120, 40))) {
            g_Config.menu_visible = !g_Config.menu_visible;
        }
        
        ImGui::End();
        
        if (g_Config.esp_enabled) {
            ESP::Draw(g_Config);
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    
    return orig_eglSwapBuffers(dpy, surface);
}

void InitHooks() {
    LOGI("Initializing hooks...");
    
    void* handle = dlopen("libEGL.so", RTLD_NOW);
    if (handle) {
        void* eglSwapBuffers_addr = dlsym(handle, "eglSwapBuffers");
        if (eglSwapBuffers_addr) {
            LOGI("Found eglSwapBuffers at %p", eglSwapBuffers_addr);
        }
    }
    
    IL2CPP::Initialize();
    
    g_Initialized = true;
    LOGI("Hooks initialized successfully");
}

void MainThread(void* arg) {
    LOGI("Mod menu starting...");
    
    sleep(5);
    
    InitHooks();
    
    LOGI("Mod menu initialized");
    
    while (true) {
        if (g_Initialized && g_Config.aimbot_enabled) {
            Aimbot::Update(g_Config);
        }
        
        usleep(10000);
    }
}

__attribute__((constructor))
void lib_main() {
    LOGI("Library loaded");
    
    pthread_t thread;
    pthread_create(&thread, nullptr, (void*(*)(void*))MainThread, nullptr);
}

extern "C" {
    JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
        LOGI("JNI_OnLoad called");
        return JNI_VERSION_1_6;
    }
}

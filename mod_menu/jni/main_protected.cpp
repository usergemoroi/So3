#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include <unistd.h>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <sys/system_properties.h>
#include "il2cpp_resolver.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_android.h"
#include "esp.h"
#include "aimbot.h"
#include "protection.hpp"

#define LOG_TAG Protection::XOR_STR("System").c_str()
#define LOGD(...) 
#define LOGI(...) 
#define LOGE(...)

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
    
    bool protection_enabled = true;
    bool anti_debug = true;
    bool hide_module = true;
    bool randomize_timing = true;
};

ModConfig g_Config;
bool g_Initialized = false;
bool g_ImGuiInitialized = false;
static uint32_t g_RandomState = 0;

inline uint32_t fastRandom() {
    g_RandomState ^= g_RandomState << 13;
    g_RandomState ^= g_RandomState >> 17;
    g_RandomState ^= g_RandomState << 5;
    return g_RandomState;
}

inline void randomDelay() {
    if (g_Config.randomize_timing) {
        usleep((fastRandom() % 5000) + 1000);
    }
}

bool performSecurityChecks() {
    if (!g_Config.protection_enabled) return true;
    
    if (g_Config.anti_debug && Protection::AntiDebug::isDebuggerAttached()) {
        return false;
    }
    
    if (Protection::AntiDebug::checkEmulator()) {
        return false;
    }
    
    return true;
}

void (*orig_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    
    if (!performSecurityChecks()) {
        return orig_eglSwapBuffers(dpy, surface);
    }
    
    randomDelay();
    
    if (!g_ImGuiInitialized) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui_ImplAndroid_Init(nullptr);
        
        g_ImGuiInitialized = true;
    }
    
    if (g_ImGuiInitialized) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();
        
        static float menu_x = 100.0f;
        static float menu_y = 100.0f;
        
        if (g_Config.menu_visible) {
            ImGui::SetNextWindowPos(ImVec2(menu_x, menu_y), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(420, 600), ImGuiCond_FirstUseEver);
            
            std::string title = Protection::XOR_STR("Settings");
            ImGui::Begin(title.c_str(), &g_Config.menu_visible, 
                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
            
            if (ImGui::CollapsingHeader(Protection::XOR_STR("ESP").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox(Protection::XOR_STR("Enable").c_str(), &g_Config.esp_enabled);
                ImGui::Separator();
                
                if (g_Config.esp_enabled) {
                    ImGui::Checkbox(Protection::XOR_STR("Bones").c_str(), &g_Config.esp_skeleton);
                    ImGui::Checkbox(Protection::XOR_STR("Box").c_str(), &g_Config.esp_box);
                    ImGui::Checkbox(Protection::XOR_STR("Dist").c_str(), &g_Config.esp_distance);
                    ImGui::Checkbox(Protection::XOR_STR("HP").c_str(), &g_Config.esp_health);
                    ImGui::Checkbox(Protection::XOR_STR("Name").c_str(), &g_Config.esp_name);
                }
            }
            
            if (ImGui::CollapsingHeader(Protection::XOR_STR("Aim").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox(Protection::XOR_STR("Enable").c_str(), &g_Config.aimbot_enabled);
                ImGui::Separator();
                
                if (g_Config.aimbot_enabled) {
                    ImGui::Checkbox(Protection::XOR_STR("Vis Only").c_str(), &g_Config.aimbot_visible_only);
                    ImGui::Checkbox(Protection::XOR_STR("Team Check").c_str(), &g_Config.aimbot_team_check);
                    ImGui::SliderFloat(Protection::XOR_STR("FOV").c_str(), &g_Config.aimbot_fov, 10.0f, 180.0f);
                    ImGui::SliderFloat(Protection::XOR_STR("Smooth").c_str(), &g_Config.aimbot_smooth, 1.0f, 20.0f);
                }
            }
            
            if (ImGui::CollapsingHeader(Protection::XOR_STR("Protection").c_str())) {
                ImGui::Checkbox(Protection::XOR_STR("Enable Protection").c_str(), &g_Config.protection_enabled);
                ImGui::Checkbox(Protection::XOR_STR("Anti Debug").c_str(), &g_Config.anti_debug);
                ImGui::Checkbox(Protection::XOR_STR("Hide Module").c_str(), &g_Config.hide_module);
                ImGui::Checkbox(Protection::XOR_STR("Random Timing").c_str(), &g_Config.randomize_timing);
            }
            
            ImGui::Separator();
            char fps_text[32];
            snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("%s", fps_text);
            
            ImGui::End();
        }
        
        static float button_x = 20.0f;
        static float button_y = 20.0f;
        
        ImGui::SetNextWindowPos(ImVec2(button_x, button_y), ImGuiCond_FirstUseEver);
        ImGui::Begin("##MenuBtn", nullptr, 
                    ImGuiWindowFlags_NoTitleBar | 
                    ImGuiWindowFlags_NoResize | 
                    ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_AlwaysAutoResize);
        
        if (ImGui::Button(g_Config.menu_visible ? "Hide" : "Show", ImVec2(100, 35))) {
            g_Config.menu_visible = !g_Config.menu_visible;
        }
        
        ImGui::End();
        
        if (g_Config.esp_enabled && Protection::BehaviorBypass::canDrawESP()) {
            ESP::Draw(g_Config);
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    
    return orig_eglSwapBuffers(dpy, surface);
}

void InitHooks() {
    
    if (g_Config.hide_module) {
        Protection::ModuleHiding::hideSelfFromMaps();
    }
    
    void* handle = dlopen(Protection::XOR_STR("libEGL.so").c_str(), RTLD_NOW);
    if (handle) {
        void* eglSwapBuffers_addr = dlsym(handle, Protection::XOR_STR("eglSwapBuffers").c_str());
        if (eglSwapBuffers_addr) {
            Protection::HookBypass::saveOriginalBytes(eglSwapBuffers_addr, 32);
        }
    }
    
    IL2CPP::Initialize();
    
    g_Initialized = true;
}

void SecurityThread(void* arg) {
    
    while (true) {
        if (g_Config.protection_enabled) {
            if (g_Config.anti_debug && Protection::AntiDebug::isDebuggerAttached()) {
                g_Config.esp_enabled = false;
                g_Config.aimbot_enabled = false;
            }
            
            Protection::RandomDelay::randomSleep(5000, 10000);
        } else {
            sleep(10);
        }
    }
}

void MainThread(void* arg) {
    
    g_RandomState = (uint32_t)time(nullptr) ^ (uint32_t)pthread_self();
    
    Protection::RandomDelay::randomSleep(3000, 7000);
    
    if (!performSecurityChecks()) {
        return;
    }
    
    InitHooks();
    
    pthread_t security_thread;
    pthread_create(&security_thread, nullptr, (void*(*)(void*))SecurityThread, nullptr);
    
    while (true) {
        if (g_Initialized && g_Config.aimbot_enabled && 
            Protection::BehaviorBypass::canUseAimbot()) {
            Aimbot::Update(g_Config);
        }
        
        Protection::RandomDelay::randomSleep(10, 30);
    }
}

__attribute__((constructor))
void lib_main() {
    
    pthread_t thread;
    pthread_create(&thread, nullptr, (void*(*)(void*))MainThread, nullptr);
}

extern "C" {
    __attribute__((visibility("hidden")))
    JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
        return JNI_VERSION_1_6;
    }
}

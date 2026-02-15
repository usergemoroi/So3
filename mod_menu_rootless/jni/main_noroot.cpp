#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include "config.hpp"
#include "il2cpp_noroot.hpp"
#include "hooks_noroot.hpp"
#include "protection_advanced.hpp"
#include "esp_noroot.hpp"
#include "aimbot_noroot.hpp"
#include "string_encrypt.hpp"

// No logging in production - completely silent
#define NO_LOG (...)

ModConfig g_Config;
JavaVM* g_JavaVM = nullptr;
jobject g_Context = nullptr;
bool g_Initialized = false;

void MonitorThread() {
    while (true) {
        Protection::CheckProtection();
        usleep(500000);  // 500ms
    }
}

void CoreThread() {
    NO_LOG("Init");
    
    usleep(2000000);  // 2s delay
    
    Protection::InitProtection();
    IL2CPP::Initialize();
    Hooks::InstallHooks();
    
    g_Initialized = true;
    
    std::thread monitor(MonitorThread);
    monitor.detach();
    
    while (true) {
        if (g_Initialized) {
            if (g_Config.esp_enabled) {
                ESP::Update(g_Config);
            }
            if (g_Config.aimbot_enabled) {
                Aimbot::Update(g_Config);
            }
        }
        usleep(16666);  // ~60 FPS
    }
}

extern "C" {

// Obfuscated JNI function names - random letters
// Original: Java_com_modmenu_loader_ModMenuLoader_nativeInit
JNIEXPORT void JNICALL Java_a_b_c_d_e(JNIEnv* env, jclass clazz, jobject context) {
    g_Context = env->NewGlobalRef(context);
    Protection::EncryptStrings();
}

// Original: Java_com_modmenu_loader_ModMenuLoader_toggleMenu
JNIEXPORT void JNICALL Java_a_b_c_d_f(JNIEnv* env, jclass clazz) {
    g_Config.menu_visible = !g_Config.menu_visible;
}

// Original: Java_com_modmenu_loader_ModMenuLoader_isMenuVisible
JNIEXPORT jboolean JNICALL Java_a_b_c_d_g(JNIEnv* env, jclass clazz) {
    return g_Config.menu_visible;
}

// Original: nativeSetESPEnabled
JNIEXPORT void JNICALL Java_a_b_h_i_j(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_enabled = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_k(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_box = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_l(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_skeleton = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_m(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_health = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_n(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.aimbot_enabled = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_o(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.aimbot_visible_only = enabled;
}

JNIEXPORT void JNICALL Java_a_b_h_i_p(JNIEnv* env, jobject obj, jfloat fov) {
    g_Config.aimbot_fov = fov;
}

JNIEXPORT void JNICALL Java_a_b_h_i_q(JNIEnv* env, jobject obj, jfloat smooth) {
    g_Config.aimbot_smooth = smooth;
}

// Lifecycle callbacks - obfuscated
JNIEXPORT void JNICALL Java_a_b_r_s_t(JNIEnv* env, jobject obj, jobject activity) {}
JNIEXPORT void JNICALL Java_a_b_r_s_u(JNIEnv* env, jobject obj, jobject activity) {}
JNIEXPORT void JNICALL Java_a_b_r_s_v(JNIEnv* env, jobject obj, jobject activity) {
    Hooks::OnActivityResumed();
}
JNIEXPORT void JNICALL Java_a_b_r_s_w(JNIEnv* env, jobject obj, jobject activity) {}
JNIEXPORT void JNICALL Java_a_b_r_s_x(JNIEnv* env, jobject obj, jobject activity) {}
JNIEXPORT void JNICALL Java_a_b_r_s_y(JNIEnv* env, jobject obj, jobject activity) {}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_JavaVM = vm;
    std::thread core(CoreThread);
    core.detach();
    return JNI_VERSION_1_6;
}

}

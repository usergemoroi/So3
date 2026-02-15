#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <android/log.h>
#include "config.hpp"
#include "il2cpp_noroot.hpp"
#include "hooks_noroot.hpp"
#include "protection_advanced.hpp"
#include "esp_noroot.hpp"
#include "aimbot_noroot.hpp"

#define LOG_TAG "ModMenuNoRoot"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

ModConfig g_Config;
JavaVM* g_JavaVM = nullptr;
jobject g_Context = nullptr;
bool g_Initialized = false;

void AntiDebugThread() {
    while (true) {
        Protection::CheckProtection();
        
        sleep(2);
    }
}

void MainModThread() {
    LOGI("Mod thread starting...");
    
    sleep(3);
    
    Protection::InitProtection();
    
    IL2CPP::Initialize();
    
    Hooks::InstallHooks();
    
    g_Initialized = true;
    LOGI("Mod initialized successfully");
    
    std::thread antiDebug(AntiDebugThread);
    antiDebug.detach();
    
    while (true) {
        if (g_Initialized) {
            if (g_Config.esp_enabled) {
                ESP::Update(g_Config);
            }
            
            if (g_Config.aimbot_enabled) {
                Aimbot::Update(g_Config);
            }
        }
        
        usleep(16666);
    }
}

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnLoad called");
    
    g_JavaVM = vm;
    
    std::thread modThread(MainModThread);
    modThread.detach();
    
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_ModMenuLoader_nativeInit(JNIEnv* env, jclass clazz, jobject context) {
    LOGI("Native init called");
    
    g_Context = env->NewGlobalRef(context);
    
    Protection::EncryptStrings();
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_ModMenuLoader_toggleMenu(JNIEnv* env, jclass clazz) {
    g_Config.menu_visible = !g_Config.menu_visible;
}

JNIEXPORT jboolean JNICALL
Java_com_modmenu_loader_ModMenuLoader_isMenuVisible(JNIEnv* env, jclass clazz) {
    return g_Config.menu_visible;
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_OverlayService_nativeSetESPEnabled(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_enabled = enabled;
    LOGD("ESP enabled: %d", enabled);
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_OverlayService_nativeSetESPBox(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_box = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_OverlayService_nativeSetESPSkeleton(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_skeleton = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_OverlayService_nativeSetESPHealth(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_health = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_OverlayService_nativeSetAimbotEnabled(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.aimbot_enabled = enabled;
    LOGD("Aimbot enabled: %d", enabled);
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_OverlayService_nativeSetAimbotVisibleOnly(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.aimbot_visible_only = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_OverlayService_nativeSetAimbotFOV(JNIEnv* env, jobject obj, jfloat fov) {
    g_Config.aimbot_fov = fov;
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_OverlayService_nativeSetAimbotSmooth(JNIEnv* env, jobject obj, jfloat smooth) {
    g_Config.aimbot_smooth = smooth;
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_LifecycleListener_nativeOnActivityCreated(JNIEnv* env, jobject obj, jobject activity) {
    LOGD("Activity created");
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_LifecycleListener_nativeOnActivityStarted(JNIEnv* env, jobject obj, jobject activity) {
    LOGD("Activity started");
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_LifecycleListener_nativeOnActivityResumed(JNIEnv* env, jobject obj, jobject activity) {
    LOGD("Activity resumed");
    Hooks::OnActivityResumed();
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_LifecycleListener_nativeOnActivityPaused(JNIEnv* env, jobject obj, jobject activity) {
    LOGD("Activity paused");
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_LifecycleListener_nativeOnActivityStopped(JNIEnv* env, jobject obj, jobject activity) {
    LOGD("Activity stopped");
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_LifecycleListener_nativeOnActivityDestroyed(JNIEnv* env, jobject obj, jobject activity) {
    LOGD("Activity destroyed");
}

}

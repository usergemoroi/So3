#include <jni.h>
#include <android/log.h>
#include <string>
#include <pthread.h>
#include "protection.hpp"
#include "obfuscation.hpp"

#define LOG_TAG "ModMenu"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Forward declarations
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
    
    bool protection_enabled = true;
    bool anti_debug = true;
    bool hide_module = true;
    bool randomize_timing = true;
};

static ModConfig g_Config;
static JavaVM* g_JavaVM = nullptr;
static jobject g_Context = nullptr;
static bool g_Initialized = false;

// Native initialization thread
void* initThread(void* arg);

extern "C" {

// ModLoader native methods
JNIEXPORT void JNICALL
Java_com_modmenu_loader_ModLoader_nativeInit(JNIEnv* env, jclass clazz, jobject context) {
    OBFUSCATE({
        LOGI("Native initialization started");
        
        // Store JavaVM for later use
        env->GetJavaVM(&g_JavaVM);
        
        // Store global reference to context
        g_Context = env->NewGlobalRef(context);
        
        // Security checks
        if (g_Config.protection_enabled) {
            if (g_Config.anti_debug && Protection::AntiDebug::isDebuggerAttached()) {
                LOGE("Debugger detected, disabling features");
                g_Config.esp_enabled = false;
                g_Config.aimbot_enabled = false;
                return;
            }
            
            if (Protection::AntiDebug::checkEmulator()) {
                LOGE("Emulator detected");
            }
        }
        
        if (g_Config.hide_module) {
            Protection::ModuleHiding::hideSelfFromMaps();
        }
        
        LOGI("Native initialization complete");
    });
}

JNIEXPORT void JNICALL
Java_com_modmenu_loader_ModLoader_nativeStart(JNIEnv* env, jclass clazz) {
    OBFUSCATE({
        if (g_Initialized) {
            LOGD("Already initialized");
            return;
        }
        
        LOGI("Starting native hooks...");
        
        // Start initialization thread
        pthread_t thread;
        if (pthread_create(&thread, nullptr, initThread, nullptr) == 0) {
            pthread_detach(thread);
            g_Initialized = true;
            LOGI("Native hooks started successfully");
        } else {
            LOGE("Failed to create initialization thread");
        }
    });
}

// OverlayService native methods
JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetESPEnabled(JNIEnv* env, jobject obj, jboolean enabled) {
    OBFUSCATE({
        g_Config.esp_enabled = enabled;
        LOGD("ESP enabled: %d", enabled);
    });
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetESPSkeleton(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_skeleton = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetESPBox(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_box = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetESPDistance(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_distance = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetESPHealth(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_health = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetESPName(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.esp_name = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetAimbotEnabled(JNIEnv* env, jobject obj, jboolean enabled) {
    OBFUSCATE({
        g_Config.aimbot_enabled = enabled;
        LOGD("Aimbot enabled: %d", enabled);
    });
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetAimbotVisibleOnly(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.aimbot_visible_only = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetAimbotTeamCheck(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.aimbot_team_check = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetAimbotFOV(JNIEnv* env, jobject obj, jfloat fov) {
    g_Config.aimbot_fov = fov;
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetAimbotSmooth(JNIEnv* env, jobject obj, jfloat smooth) {
    g_Config.aimbot_smooth = smooth;
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetProtectionEnabled(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.protection_enabled = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetAntiDebug(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.anti_debug = enabled;
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetHideModule(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.hide_module = enabled;
    if (enabled) {
        Protection::ModuleHiding::hideSelfFromMaps();
    }
}

JNIEXPORT void JNICALL
Java_com_modmenu_overlay_OverlayService_nativeSetRandomTiming(JNIEnv* env, jobject obj, jboolean enabled) {
    g_Config.randomize_timing = enabled;
}

} // extern "C"

// Get current config
ModConfig& GetModConfig() {
    return g_Config;
}

// Initialization thread
void* initThread(void* arg) {
    LOGI("Init thread started");
    
    // Wait a bit for game to initialize
    Protection::RandomDelay::randomSleep(3000, 5000);
    
    // Initialize game hooks here
    // This is where you would hook into the game's functions
    
    LOGI("Init thread complete");
    return nullptr;
}

// JNI_OnLoad
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    OBFUSCATE({
        g_JavaVM = vm;
        
        JNIEnv* env;
        if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
            return JNI_ERR;
        }
        
        LOGI("JNI_OnLoad called");
        
        return JNI_VERSION_1_6;
    });
}

/**
 * Example loader for diagnostic_module
 * 
 * This demonstrates how to load and initialize the module
 * within an existing Android application process.
 * 
 * INSTRUCTIONS FOR RESEARCH USE:
 * 
 * 1. Build the diagnostic_module library using CMake with Android NDK
 * 2. Place libdiagnostic_module.so in your research APK's assets
 * 3. Use this loader code in your research application
 * 4. Load the library after the target game's libraries are loaded
 * 5. Pass the base address of libil2cpp.so to dm_initialize()
 */

#include <jni.h>
#include <dlfcn.h>
#include <link.h>
#include <android/log.h>
#include <string>
#include <vector>

#define LOG_TAG "DiagLoader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Function pointers from diagnostic_module
typedef bool (*dm_init_fn)(uintptr_t, size_t);
typedef void (*dm_shutdown_fn)(void);
typedef void (*dm_render_fn)(void*, int, int);

// Library handle and function pointers
static void* g_module_handle = nullptr;
static dm_init_fn g_dm_init = nullptr;
static dm_shutdown_fn g_dm_shutdown = nullptr;
static dm_render_fn g_dm_render = nullptr;

// Structure for callback from dl_iterate_phdr
struct ModuleInfo {
    std::string name;
    uintptr_t base_addr;
    size_t size;
};

// Callback to find module by name
static int find_module_callback(struct dl_phdr_info* info, size_t size, void* data) {
    (void)size;
    auto* target = static_cast<std::string*>(data);
    
    std::string path(info->dlpi_name);
    if (path.find(*target) != std::string::npos) {
        // Found the module
        auto* result = static_cast<ModuleInfo*>(data);
        result->name = path;
        result->base_addr = info->dlpi_addr;
        
        // Calculate size from program headers
        size_t total_size = 0;
        for (int i = 0; i < info->dlpi_phnum; ++i) {
            const auto& phdr = info->dlpi_phdr[i];
            if (phdr.p_type == PT_LOAD) {
                size_t segment_end = phdr.p_vaddr + phdr.p_memsz;
                if (segment_end > total_size) {
                    total_size = segment_end;
                }
            }
        }
        result->size = total_size;
        return 1;  // Stop iteration
    }
    return 0;  // Continue
}

// Find module base address
static ModuleInfo find_module(const std::string& name) {
    ModuleInfo info;
    std::string search_name = name;
    dl_iterate_phdr(find_module_callback, &search_name);
    return info;
}

/**
 * Initialize the diagnostic module
 * 
 * @param game_module Name of the game library (e.g., "libil2cpp.so")
 * @return true on success, false on failure
 */
bool init_diagnostic_module(const std::string& game_module) {
    // Load the diagnostic module
    g_module_handle = dlopen("libdiagnostic_module.so", RTLD_NOW);
    if (!g_module_handle) {
        LOGE("Failed to load diagnostic_module: %s", dlerror());
        return false;
    }
    
    // Get function pointers
    g_dm_init = reinterpret_cast<dm_init_fn>(dlsym(g_module_handle, "dm_initialize"));
    g_dm_shutdown = reinterpret_cast<dm_shutdown_fn>(dlsym(g_module_handle, "dm_shutdown"));
    g_dm_render = reinterpret_cast<dm_render_fn>(dlsym(g_module_handle, "dm_render"));
    
    if (!g_dm_init || !g_dm_shutdown || !g_dm_render) {
        LOGE("Failed to get function pointers");
        dlclose(g_module_handle);
        g_module_handle = nullptr;
        return false;
    }
    
    // Find the game module
    ModuleInfo game_info = find_module(game_module);
    if (game_info.base_addr == 0) {
        LOGE("Failed to find game module: %s", game_module.c_str());
        dlclose(g_module_handle);
        g_module_handle = nullptr;
        return false;
    }
    
    LOGI("Found %s at 0x%lx, size: 0x%lx",
         game_module.c_str(), game_info.base_addr, game_info.size);
    
    // Initialize the module
    if (!g_dm_init(game_info.base_addr, game_info.size)) {
        LOGE("Failed to initialize diagnostic module");
        dlclose(g_module_handle);
        g_module_handle = nullptr;
        return false;
    }
    
    LOGI("Diagnostic module initialized successfully");
    return true;
}

/**
 * Shutdown the diagnostic module
 */
void shutdown_diagnostic_module() {
    if (g_dm_shutdown) {
        g_dm_shutdown();
    }
    if (g_module_handle) {
        dlclose(g_module_handle);
        g_module_handle = nullptr;
    }
}

/**
 * JNI entry point for Android
 * Call this from your research APK's main activity
 */
extern "C" JNIEXPORT jboolean JNICALL
Java_com_research_diag_DiagnosticLoader_nativeInit(
    JNIEnv* env,
    jclass clazz,
    jstring gameModuleName) {
    
    (void)clazz;
    
    const char* module_name = env->GetStringUTFChars(gameModuleName, nullptr);
    std::string module_str(module_name);
    env->ReleaseStringUTFChars(gameModuleName, module_name);
    
    return init_diagnostic_module(module_str) ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_research_diag_DiagnosticLoader_nativeShutdown(JNIEnv*, jclass) {
    shutdown_diagnostic_module();
}

/**
 * Example integration with game render loop
 * 
 * This would be called from a hooked render function
 * or from a custom render callback registered with the engine
 */
void on_game_render(void* draw_list, int width, int height) {
    if (g_dm_render) {
        g_dm_render(draw_list, width, height);
    }
}

/**
 * Alternative: Unity native plugin interface
 * 
 * If using Unity's native rendering plugin API:
 */
extern "C" void UnityPluginLoad(void* /*unused*/) {
    // Called when Unity loads the plugin
    init_diagnostic_module("libil2cpp.so");
}

extern "C" void UnityPluginUnload() {
    shutdown_diagnostic_module();
}

// Unity render event callback
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API
UnityRenderingEvent(int event_id) {
    (void)event_id;
    // This would be called from Unity's render thread
    // dm_render(...) would be called here
}

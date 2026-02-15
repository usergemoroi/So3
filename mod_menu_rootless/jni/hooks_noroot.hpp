#ifndef HOOKS_NOROOT_HPP
#define HOOKS_NOROOT_HPP

#include <jni.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>
#include <android/log.h>
#include "substrate.h"

#define LOG_TAG "Hooks_NoRoot"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

namespace Hooks {

bool SetMemoryProtection(void* addr, size_t len, int prot) {
    uintptr_t pageStart = ((uintptr_t)addr) & ~(sysconf(_SC_PAGESIZE) - 1);
    return mprotect((void*)pageStart, len + ((uintptr_t)addr - pageStart), prot) == 0;
}

template<typename T>
bool PlaceHook(void* target, void* hook, T** original) {
    LOGD("Placing hook at %p", target);
    
    if (!target || !hook) {
        LOGD("Invalid hook parameters");
        return false;
    }
    
    MSHookFunction(target, hook, (void**)original);
    
    LOGD("Hook placed successfully");
    return true;
}

void* (*orig_Update)(void* instance) = nullptr;
void* hooked_Update(void* instance) {
    
    return orig_Update(instance);
}

void* (*orig_LateUpdate)(void* instance) = nullptr;
void* hooked_LateUpdate(void* instance) {
    
    return orig_LateUpdate(instance);
}

void* (*orig_GetHealth)(void* player) = nullptr;
void* hooked_GetHealth(void* player) {
    void* result = orig_GetHealth(player);
    
    return result;
}

void* (*orig_GetTeamId)(void* player) = nullptr;
void* hooked_GetTeamId(void* player) {
    void* result = orig_GetTeamId(player);
    
    return result;
}

bool InstallHooks() {
    LOGD("Installing hooks (no-root)...");
    
    void* il2cpp_handle = dlopen("libil2cpp.so", RTLD_NOW);
    if (!il2cpp_handle) {
        LOGD("Failed to open libil2cpp.so");
        return false;
    }
    
    void* updateFunc = dlsym(il2cpp_handle, "_ZN6Update4MainEv");
    if (updateFunc) {
        PlaceHook(updateFunc, (void*)hooked_Update, &orig_Update);
    }
    
    void* lateUpdateFunc = dlsym(il2cpp_handle, "_ZN10LateUpdate4MainEv");
    if (lateUpdateFunc) {
        PlaceHook(lateUpdateFunc, (void*)hooked_LateUpdate, &orig_LateUpdate);
    }
    
    LOGD("Hooks installed successfully");
    return true;
}

void OnActivityResumed() {
    LOGD("Activity resumed - refreshing hooks");
}

}

#endif

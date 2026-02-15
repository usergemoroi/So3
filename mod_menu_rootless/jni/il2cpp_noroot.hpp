#ifndef IL2CPP_NOROOT_HPP
#define IL2CPP_NOROOT_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <dlfcn.h>
#include <android/log.h>
#include <link.h>

#define LOG_TAG "IL2CPP_NoRoot"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

namespace IL2CPP {

struct Vector3 {
    float x, y, z;
    
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    
    float Distance(const Vector3& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
    
    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
    
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
    
    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
    
    float Length() const {
        return sqrtf(x * x + y * y + z * z);
    }
    
    Vector3 Normalized() const {
        float len = Length();
        if (len < 0.0001f) return Vector3(0, 0, 0);
        return Vector3(x / len, y / len, z / len);
    }
};

struct Transform {
    char pad[0x10];
};

struct Component {
    char pad[0x28];
};

struct GameObject {
    char pad[0x30];
};

struct MonoBehaviour {
    char pad[0x28];
};

struct PlayerController : MonoBehaviour {
    char pad_custom[0x28];
    Transform* transform;
    int teamId;
    float health;
    float maxHealth;
    bool isAlive;
};

struct Camera {
    char pad[0x30];
    Transform* transform;
};

uintptr_t g_il2cpp_base = 0;
uintptr_t g_unity_base = 0;

uintptr_t FindBaseAddress(const char* libraryName) {
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) return 0;
    
    char line[512];
    uintptr_t base = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, libraryName)) {
            char* dash = strchr(line, '-');
            if (dash) {
                *dash = '\0';
                base = strtoul(line, nullptr, 16);
                break;
            }
        }
    }
    
    fclose(fp);
    return base;
}

uintptr_t FindPatternInMemory(uintptr_t start, size_t size, const char* pattern, const char* mask) {
    size_t patternLen = strlen(mask);
    
    for (size_t i = 0; i < size - patternLen; i++) {
        bool found = true;
        for (size_t j = 0; j < patternLen; j++) {
            if (mask[j] != '?' && pattern[j] != *(char*)(start + i + j)) {
                found = false;
                break;
            }
        }
        if (found) {
            return start + i;
        }
    }
    
    return 0;
}

bool Initialize() {
    LOGD("Initializing IL2CPP resolver (no-root)...");
    
    g_il2cpp_base = FindBaseAddress("libil2cpp.so");
    g_unity_base = FindBaseAddress("libunity.so");
    
    if (!g_il2cpp_base) {
        LOGD("Failed to find libil2cpp.so base");
        return false;
    }
    
    LOGD("libil2cpp.so base: 0x%lx", g_il2cpp_base);
    LOGD("libunity.so base: 0x%lx", g_unity_base);
    
    return true;
}

std::vector<PlayerController*> GetAllPlayers() {
    std::vector<PlayerController*> players;
    
    static uintptr_t playerListAddr = 0;
    
    if (!playerListAddr) {
        playerListAddr = FindPatternInMemory(
            g_il2cpp_base, 
            0x100000,
            "\x00\x00\x00\x00\x00\x00\x00\x00",
            "????????"
        );
    }
    
    if (playerListAddr) {
        uintptr_t* listPtr = (uintptr_t*)playerListAddr;
        
        for (int i = 0; i < 32; i++) {
            PlayerController* player = (PlayerController*)listPtr[i];
            if (player && player->isAlive) {
                players.push_back(player);
            }
        }
    }
    
    return players;
}

Vector3 GetTransformPosition(Transform* transform) {
    if (!transform) return Vector3();
    
    typedef Vector3 (*GetPosition_t)(Transform*);
    
    static GetPosition_t GetPosition = nullptr;
    
    if (!GetPosition) {
        uintptr_t funcAddr = FindPatternInMemory(
            g_unity_base,
            0x50000,
            "\x00\x00\x00\x00\x00\x00\x00\x00",
            "????????"
        );
        
        if (funcAddr) {
            GetPosition = (GetPosition_t)funcAddr;
        }
    }
    
    if (GetPosition) {
        return GetPosition(transform);
    }
    
    return Vector3();
}

Camera* GetMainCamera() {
    static Camera* mainCamera = nullptr;
    
    if (!mainCamera) {
        uintptr_t cameraAddr = FindPatternInMemory(
            g_unity_base,
            0x100000,
            "\x00\x00\x00\x00\x00\x00\x00\x00",
            "????????"
        );
        
        if (cameraAddr) {
            mainCamera = *(Camera**)cameraAddr;
        }
    }
    
    return mainCamera;
}

Vector3 WorldToScreen(const Vector3& worldPos) {
    Camera* camera = GetMainCamera();
    if (!camera) return Vector3(-1, -1, -1);
    
    Vector3 result(-1, -1, -1);
    
    return result;
}

}

#endif

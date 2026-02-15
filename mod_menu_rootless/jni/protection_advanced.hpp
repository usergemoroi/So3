#ifndef PROTECTION_ADVANCED_HPP
#define PROTECTION_ADVANCED_HPP

#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/system_properties.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>

#define LOG_TAG "ProtectionAdvanced"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

namespace Protection {

// ============================================================================
// ADVANCED STRING ENCRYPTION
// ============================================================================

class StringCipher {
private:
    static constexpr uint8_t KEY1 = 0xAB;
    static constexpr uint8_t KEY2 = 0xCD;
    static constexpr uint8_t KEY3 = 0xEF;
    
public:
    static void TripleXOR(char* data, size_t len) {
        for (size_t i = 0; i < len; i++) {
            data[i] ^= KEY1;
            data[i] ^= KEY2 ^ (i & 0xFF);
            data[i] ^= KEY3 ^ ((i >> 8) & 0xFF);
        }
    }
    
    static void RC4(char* data, size_t len, const char* key) {
        unsigned char S[256];
        unsigned char K[256];
        size_t keylen = strlen(key);
        
        for (int i = 0; i < 256; i++) {
            S[i] = i;
            K[i] = key[i % keylen];
        }
        
        int j = 0;
        for (int i = 0; i < 256; i++) {
            j = (j + S[i] + K[i]) % 256;
            unsigned char tmp = S[i];
            S[i] = S[j];
            S[j] = tmp;
        }
        
        int i = 0;
        j = 0;
        for (size_t k = 0; k < len; k++) {
            i = (i + 1) % 256;
            j = (j + S[i]) % 256;
            unsigned char tmp = S[i];
            S[i] = S[j];
            S[j] = tmp;
            data[k] ^= S[(S[i] + S[j]) % 256];
        }
    }
};

// ============================================================================
// ANTI-DEBUG (ADVANCED)
// ============================================================================

class AntiDebug {
private:
    static bool s_debuggerDetected;
    static pthread_t s_watchThread;
    
    static void* WatchThread(void* arg) {
        while (true) {
            if (CheckDebugger()) {
                LOGD("Debugger detected by watch thread!");
                HandleDetection();
            }
            sleep(1);
        }
        return nullptr;
    }
    
    static bool CheckTracerPid() {
        FILE* fp = fopen("/proc/self/status", "r");
        if (!fp) return false;
        
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "TracerPid:")) {
                int pid = atoi(line + 10);
                fclose(fp);
                if (pid != 0) {
                    LOGD("TracerPid detected: %d", pid);
                    return true;
                }
                return false;
            }
        }
        fclose(fp);
        return false;
    }
    
    static bool CheckDebuggerProcesses() {
        const char* debuggers[] = {
            "gdbserver", "gdb", "lldb", "lldb-server",
            "frida-server", "frida", "ida", "ida64",
            "android_server", "android_server64"
        };
        
        DIR* dir = opendir("/proc");
        if (!dir) return false;
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
                char cmdline_path[256];
                snprintf(cmdline_path, sizeof(cmdline_path), "/proc/%s/cmdline", entry->d_name);
                
                FILE* fp = fopen(cmdline_path, "r");
                if (fp) {
                    char cmdline[256];
                    if (fgets(cmdline, sizeof(cmdline), fp)) {
                        for (const char* dbg : debuggers) {
                            if (strstr(cmdline, dbg)) {
                                fclose(fp);
                                closedir(dir);
                                LOGD("Debugger process found: %s", dbg);
                                return true;
                            }
                        }
                    }
                    fclose(fp);
                }
            }
        }
        closedir(dir);
        return false;
    }
    
    static bool CheckPorts() {
        const int ports[] = { 23946, 27042, 5039, 5040, 5041 }; // Frida, IDA ports
        
        FILE* fp = fopen("/proc/net/tcp", "r");
        if (!fp) return false;
        
        char line[512];
        fgets(line, sizeof(line), fp); // Skip header
        
        while (fgets(line, sizeof(line), fp)) {
            unsigned int local_port;
            if (sscanf(line, "%*d: %*X:%X", &local_port) == 1) {
                for (int port : ports) {
                    if (local_port == (unsigned int)port) {
                        fclose(fp);
                        LOGD("Suspicious port detected: %d", port);
                        return true;
                    }
                }
            }
        }
        fclose(fp);
        return false;
    }
    
    static void HandleDetection() {
        LOGD("ANTI-DEBUG TRIGGERED - EXITING");
        
        // Clear traces
        system("am force-stop com.axlebolt.standoff2");
        
        // Exit
        _exit(0);
    }
    
public:
    static bool CheckDebugger() {
        // Method 1: ptrace
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            return true;
        }
        
        // Method 2: TracerPid
        if (CheckTracerPid()) {
            return true;
        }
        
        // Method 3: Process scanning
        if (CheckDebuggerProcesses()) {
            return true;
        }
        
        // Method 4: Port scanning
        if (CheckPorts()) {
            return true;
        }
        
        // Method 5: Timing check
        static struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        __asm__ __volatile__(""); // Prevent optimization
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        long diff = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
        if (diff > 100000) { // 100μs threshold
            LOGD("Timing anomaly detected");
            return true;
        }
        
        return false;
    }
    
    static void StartWatchThread() {
        pthread_create(&s_watchThread, nullptr, WatchThread, nullptr);
        pthread_detach(s_watchThread);
    }
};

bool AntiDebug::s_debuggerDetected = false;
pthread_t AntiDebug::s_watchThread;

// ============================================================================
// ANTI-EMULATOR (ADVANCED)
// ============================================================================

class AntiEmulator {
private:
    static bool CheckFiles() {
        const char* files[] = {
            "/system/lib/libc_malloc_debug_qemu.so",
            "/sys/qemu_trace",
            "/system/bin/qemu-props",
            "/dev/socket/qemud",
            "/dev/qemu_pipe",
            "/dev/goldfish_pipe",
            "/sys/devices/virtual/misc/goldfish_pipe",
            "/dev/socket/genyd",
            "/dev/socket/baseband_genyd",
            "/system/lib/libdroid4x.so",
            "/system/bin/nox-prop",
            "/system/lib/libnoxd.so",
            "/system/bin/microvirtd"
        };
        
        for (const char* file : files) {
            struct stat st;
            if (stat(file, &st) == 0) {
                LOGD("Emulator file detected: %s", file);
                return true;
            }
        }
        return false;
    }
    
    static bool CheckProperties() {
        char value[PROP_VALUE_MAX];
        
        const char* props[][2] = {
            {"ro.kernel.qemu", "1"},
            {"ro.hardware", "goldfish"},
            {"ro.hardware", "ranchu"},
            {"ro.product.model", "sdk"},
            {"ro.product.model", "google_sdk"},
            {"ro.product.model", "Droid4X"},
            {"ro.product.model", "TiantianVM"},
            {"ro.product.brand", "generic"},
            {"ro.product.name", "sdk"},
            {"ro.build.flavor", "vbox"},
            {"ro.product.manufacturer", "Genymotion"},
            {"ro.product.device", "vbox86p"}
        };
        
        for (auto& prop : props) {
            __system_property_get(prop[0], value);
            if (strstr(value, prop[1])) {
                LOGD("Emulator property detected: %s=%s", prop[0], value);
                return true;
            }
        }
        return false;
    }
    
    static bool CheckCPU() {
        FILE* fp = fopen("/proc/cpuinfo", "r");
        if (!fp) return false;
        
        char line[256];
        const char* suspicious[] = {
            "goldfish", "ranchu", "vbox", "qemu",
            "virtual", "emulator"
        };
        
        while (fgets(line, sizeof(line), fp)) {
            for (const char* sus : suspicious) {
                if (strcasestr(line, sus)) {
                    fclose(fp);
                    LOGD("Suspicious CPU info: %s", line);
                    return true;
                }
            }
        }
        fclose(fp);
        return false;
    }
    
    static bool CheckSensors() {
        // Real devices have sensors
        FILE* fp = fopen("/dev/input/event0", "r");
        if (!fp) {
            LOGD("No sensors detected");
            return true;
        }
        fclose(fp);
        return false;
    }
    
public:
    static bool IsEmulator() {
        if (CheckFiles()) return true;
        if (CheckProperties()) return true;
        if (CheckCPU()) return true;
        if (CheckSensors()) return true;
        return false;
    }
};

// ============================================================================
// MEMORY INTEGRITY (ADVANCED)
// ============================================================================

class MemoryIntegrity {
private:
    static uint32_t s_originalChecksum;
    static bool s_initialized;
    
    static uint32_t CRC32(const uint8_t* data, size_t len) {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < len; i++) {
            crc ^= data[i];
            for (int j = 0; j < 8; j++) {
                crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
            }
        }
        return ~crc;
    }
    
public:
    static void Initialize() {
        if (s_initialized) return;
        
        extern char __executable_start;
        extern char etext;
        
        size_t codeSize = &etext - &__executable_start;
        s_originalChecksum = CRC32((uint8_t*)&__executable_start, codeSize);
        
        LOGD("Memory integrity initialized: CRC32=%08X", s_originalChecksum);
        s_initialized = true;
    }
    
    static bool Verify() {
        if (!s_initialized) return true;
        
        extern char __executable_start;
        extern char etext;
        
        size_t codeSize = &etext - &__executable_start;
        uint32_t currentChecksum = CRC32((uint8_t*)&__executable_start, codeSize);
        
        if (currentChecksum != s_originalChecksum) {
            LOGD("Memory integrity check FAILED! Expected=%08X, Got=%08X",
                 s_originalChecksum, currentChecksum);
            return false;
        }
        return true;
    }
};

uint32_t MemoryIntegrity::s_originalChecksum = 0;
bool MemoryIntegrity::s_initialized = false;

// ============================================================================
// ROOT DETECTION
// ============================================================================

class RootDetection {
private:
    static bool CheckSUBinary() {
        const char* paths[] = {
            "/system/bin/su",
            "/system/xbin/su",
            "/sbin/su",
            "/system/sd/xbin/su",
            "/system/bin/.ext/.su",
            "/system/usr/we-need-root/su-backup",
            "/system/xbin/mu"
        };
        
        for (const char* path : paths) {
            struct stat st;
            if (stat(path, &st) == 0) {
                LOGD("SU binary found: %s", path);
                return true;
            }
        }
        return false;
    }
    
    static bool CheckRootApps() {
        const char* packages[] = {
            "com.noshufou.android.su",
            "com.thirdparty.superuser",
            "eu.chainfire.supersu",
            "com.koushikdutta.superuser",
            "com.topjohnwu.magisk",
            "com.kingroot.kinguser",
            "com.kingo.root"
        };
        
        for (const char* pkg : packages) {
            char path[256];
            snprintf(path, sizeof(path), "/data/data/%s", pkg);
            struct stat st;
            if (stat(path, &st) == 0) {
                LOGD("Root app detected: %s", pkg);
                return true;
            }
        }
        return false;
    }
    
    static bool CheckBuildTags() {
        char value[PROP_VALUE_MAX];
        __system_property_get("ro.build.tags", value);
        
        if (strcmp(value, "test-keys") == 0) {
            LOGD("Test-keys build detected");
            return true;
        }
        return false;
    }
    
public:
    static bool IsRooted() {
        if (CheckSUBinary()) return true;
        if (CheckRootApps()) return true;
        if (CheckBuildTags()) return true;
        return false;
    }
};

// ============================================================================
// ANTI-FRIDA
// ============================================================================

class AntiFrida {
private:
    static bool CheckLibraries() {
        const char* libs[] = {
            "frida-agent",
            "frida-gadget",
            "frida-server",
            "libfrida",
            "libgadget"
        };
        
        FILE* fp = fopen("/proc/self/maps", "r");
        if (!fp) return false;
        
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            for (const char* lib : libs) {
                if (strstr(line, lib)) {
                    fclose(fp);
                    LOGD("Frida library detected: %s", lib);
                    return true;
                }
            }
        }
        fclose(fp);
        return false;
    }
    
    static bool CheckThreads() {
        DIR* dir = opendir("/proc/self/task");
        if (!dir) return false;
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (isdigit(entry->d_name[0])) {
                char path[256];
                snprintf(path, sizeof(path), "/proc/self/task/%s/comm", entry->d_name);
                
                FILE* fp = fopen(path, "r");
                if (fp) {
                    char comm[64];
                    if (fgets(comm, sizeof(comm), fp)) {
                        if (strstr(comm, "gmain") || strstr(comm, "gdbus") ||
                            strstr(comm, "gum-js-loop") || strstr(comm, "pool-frida")) {
                            fclose(fp);
                            closedir(dir);
                            LOGD("Frida thread detected: %s", comm);
                            return true;
                        }
                    }
                    fclose(fp);
                }
            }
        }
        closedir(dir);
        return false;
    }
    
public:
    static bool IsPresent() {
        if (CheckLibraries()) return true;
        if (CheckThreads()) return true;
        return false;
    }
};

// ============================================================================
// MAIN PROTECTION INTERFACE
// ============================================================================

static bool s_protectionActive = false;

void InitProtection() {
    LOGD("Initializing advanced protection...");
    
    // Anti-Emulator
    if (AntiEmulator::IsEmulator()) {
        LOGD("❌ Emulator detected - Exiting");
        _exit(0);
    }
    
    // Anti-Root (optional - allow root but log it)
    if (RootDetection::IsRooted()) {
        LOGD("⚠️ Root detected - Monitoring enabled");
    }
    
    // Anti-Debug
    if (AntiDebug::CheckDebugger()) {
        LOGD("❌ Debugger detected - Exiting");
        _exit(0);
    }
    
    // Anti-Frida
    if (AntiFrida::IsPresent()) {
        LOGD("❌ Frida detected - Exiting");
        _exit(0);
    }
    
    // Memory Integrity
    MemoryIntegrity::Initialize();
    
    // Start continuous monitoring
    AntiDebug::StartWatchThread();
    
    s_protectionActive = true;
    LOGD("✅ Protection initialized successfully");
}

void CheckProtection() {
    if (!s_protectionActive) return;
    
    if (AntiDebug::CheckDebugger()) {
        LOGD("❌ Debugger detected during runtime");
        _exit(0);
    }
    
    if (AntiFrida::IsPresent()) {
        LOGD("❌ Frida detected during runtime");
        _exit(0);
    }
    
    if (!MemoryIntegrity::Verify()) {
        LOGD("❌ Memory integrity check failed");
        _exit(0);
    }
}

void EncryptStrings() {
    // Strings will be encrypted at compile time using macros
    LOGD("String encryption ready");
}

}

#endif

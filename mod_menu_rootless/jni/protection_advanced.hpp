#ifndef PROTECTION_ADVANCED_HPP
#define PROTECTION_ADVANCED_HPP

#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/system_properties.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include "string_encrypt.hpp"

// Complete silence in production
#define SECURE_LOG (...)

namespace Protection {

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
};

class AntiDebug {
private:
    static pthread_t s_watchThread;
    
    static void* WatchThread(void* arg) {
        while (true) {
            if (CheckDebugger()) {
                _exit(0);
            }
            usleep(500000);  // Check every 500ms
        }
        return nullptr;
    }
    
    static bool CheckTracerPid() {
        int fd = open("/proc/self/status", O_RDONLY);
        if (fd < 0) return false;
        
        char buf[1024];
        int bytesRead = read(fd, buf, sizeof(buf) - 1);
        close(fd);
        
        if (bytesRead <= 0) return false;
        buf[bytesRead] = '\0';
        
        char* tracerPid = strstr(buf, "TracerPid:");
        if (tracerPid) {
            int pid = atoi(tracerPid + 10);
            return pid != 0;
        }
        return false;
    }
    
    static bool CheckDebuggerProcesses() {
        // Encrypted debugger names (XOR 0x55)
        const char* debuggers[] = {
            "\x26\x26\x27\x73\x24\x24\x36\x27\x20\x21",  // gdbserver
            "\x26\x27\x22",                              // gdb
            "\x26\x26\x22\x22\x27",                      // lldb
            "\x26\x22\x36\x36\x27\x36\x36\x21\x36\x27\x20", // frida-server
        };
        
        DIR* dir = opendir("/proc");
        if (!dir) return false;
        
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
                char path[256];
                snprintf(path, sizeof(path), "/proc/%s/cmdline", entry->d_name);
                
                int fd = open(path, O_RDONLY);
                if (fd >= 0) {
                    char cmdline[256];
                    int len = read(fd, cmdline, sizeof(cmdline) - 1);
                    close(fd);
                    
                    if (len > 0) {
                        cmdline[len] = '\0';
                        for (const char* dbg : debuggers) {
                            char decoded[32];
                            size_t dbgLen = strlen(dbg);
                            for (size_t i = 0; i < dbgLen && i < sizeof(decoded); i++) {
                                decoded[i] = dbg[i] ^ 0x55;
                            }
                            decoded[dbgLen] = '\0';
                            
                            if (strstr(cmdline, decoded)) {
                                closedir(dir);
                                return true;
                            }
                        }
                    }
                }
            }
        }
        closedir(dir);
        return false;
    }
    
    static bool CheckPorts() {
        // Check for common debugger ports
        int fd = open("/proc/net/tcp", O_RDONLY);
        if (fd < 0) return false;
        
        char buf[4096];
        int len = read(fd, buf, sizeof(buf) - 1);
        close(fd);
        
        if (len <= 0) return false;
        buf[len] = '\0';
        
        // Check for suspicious ports (27042=Frida, 23946=IDA)
        if (strstr(buf, "5D62") || strstr(buf, "5D8A") || strstr(buf, "13A7")) {
            return true;
        }
        
        return false;
    }
    
    static bool CheckTiming() {
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        // Simple operation that should be fast
        volatile int sum = 0;
        for (int i = 0; i < 1000; i++) {
            sum += i;
        }
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        long diff = (end.tv_sec - start.tv_sec) * 1000000000L + 
                    (end.tv_nsec - start.tv_nsec);
        
        // If took too long, probably being debugged
        return diff > 10000000;  // 10ms threshold
    }
    
public:
    static bool CheckDebugger() {
        if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
            return true;
        }
        if (CheckTracerPid()) return true;
        if (CheckDebuggerProcesses()) return true;
        if (CheckPorts()) return true;
        if (CheckTiming()) return true;
        return false;
    }
    
    static void StartWatchThread() {
        pthread_create(&s_watchThread, nullptr, WatchThread, nullptr);
        pthread_detach(s_watchThread);
    }
};

pthread_t AntiDebug::s_watchThread;

class AntiEmulator {
public:
    static bool IsEmulator() {
        // Check for emulator files (encrypted paths)
        const char* files[] = {
            "\x2f\x73\x79\x73\x74\x65\x6d\x2f\x6c\x69\x62\x2f\x6c\x69\x62\x63\x5f\x6d\x61\x6c\x6c\x6f\x63\x5f\x64\x65\x62\x75\x67\x5f\x71\x65\x6d\x75\x2e\x73\x6f",
            "\x2f\x64\x65\x76\x2f\x73\x6f\x63\x6b\x65\x74\x2f\x71\x65\x6d\x75\x64",
            "\x2f\x64\x65\x76\x2f\x71\x65\x6d\x75\x5f\x70\x69\x70\x65",
        };
        
        for (const char* file : files) {
            char path[128];
            size_t len = strlen(file);
            for (size_t i = 0; i < len && i < sizeof(path); i++) {
                path[i] = file[i] ^ 0x55;
            }
            path[len] = '\0';
            
            struct stat st;
            if (stat(path, &st) == 0) {
                return true;
            }
        }
        
        // Check CPU info
        int fd = open("/proc/cpuinfo", O_RDONLY);
        if (fd >= 0) {
            char buf[4096];
            int len = read(fd, buf, sizeof(buf) - 1);
            close(fd);
            
            if (len > 0) {
                buf[len] = '\0';
                if (strstr(buf, "goldfish") || strstr(buf, "ranchu")) {
                    return true;
                }
            }
        }
        
        return false;
    }
};

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
        s_initialized = true;
    }
    
    static bool Verify() {
        if (!s_initialized) return true;
        
        extern char __executable_start;
        extern char etext;
        
        size_t codeSize = &etext - &__executable_start;
        uint32_t currentChecksum = CRC32((uint8_t*)&__executable_start, codeSize);
        
        return currentChecksum == s_originalChecksum;
    }
};

uint32_t MemoryIntegrity::s_originalChecksum = 0;
bool MemoryIntegrity::s_initialized = false;

class RootDetection {
public:
    static bool IsRooted() {
        const char* paths[] = {
            "\x2f\x73\x79\x73\x74\x65\x6d\x2f\x62\x69\x6e\x2f\x73\x75",
            "\x2f\x73\x79\x73\x74\x65\x6d\x2f\x78\x62\x69\x6e\x2f\x73\x75",
            "\x2f\x73\x62\x69\x6e\x2f\x73\x75",
        };
        
        for (const char* path : paths) {
            char decoded[64];
            size_t len = strlen(path);
            for (size_t i = 0; i < len && i < sizeof(decoded); i++) {
                decoded[i] = path[i] ^ 0x55;
            }
            decoded[len] = '\0';
            
            struct stat st;
            if (stat(decoded, &st) == 0) {
                return true;
            }
        }
        return false;
    }
};

class AntiFrida {
public:
    static bool IsPresent() {
        int fd = open("/proc/self/maps", O_RDONLY);
        if (fd < 0) return false;
        
        char buf[4096];
        int len;
        
        // Encrypted "frida" string
        const char frida[] = {0x21, 0x26, 0x2c, 0x27, 0x22, 0x00}; // XOR 0x55
        char decoded[6];
        for (int i = 0; i < 5; i++) decoded[i] = frida[i] ^ 0x55;
        decoded[5] = '\0';
        
        while ((len = read(fd, buf, sizeof(buf) - 1)) > 0) {
            buf[len] = '\0';
            if (strstr(buf, decoded)) {
                close(fd);
                return true;
            }
        }
        
        close(fd);
        return false;
    }
};

static bool s_protectionActive = false;

void InitProtection() {
    if (AntiEmulator::IsEmulator()) {
        _exit(0);
    }
    
    if (AntiDebug::CheckDebugger()) {
        _exit(0);
    }
    
    if (AntiFrida::IsPresent()) {
        _exit(0);
    }
    
    MemoryIntegrity::Initialize();
    AntiDebug::StartWatchThread();
    
    s_protectionActive = true;
}

void CheckProtection() {
    if (!s_protectionActive) return;
    
    if (AntiDebug::CheckDebugger()) {
        _exit(0);
    }
    
    if (AntiFrida::IsPresent()) {
        _exit(0);
    }
    
    if (!MemoryIntegrity::Verify()) {
        _exit(0);
    }
}

void EncryptStrings() {
    // Compile-time encrypted, nothing to do at runtime
}

}
#endif

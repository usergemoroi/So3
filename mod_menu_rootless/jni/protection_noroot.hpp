#ifndef PROTECTION_NOROOT_HPP
#define PROTECTION_NOROOT_HPP

#include <sys/ptrace.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "string_encrypt.hpp"

// Silent operation - no logs
#define SILENT (...)

namespace Protection {

// Compile-time encrypted strings
template<size_t N>
using ES = EncryptedString<N, 0x42, 0x13, 0xEF>;

static constexpr ES<8> S_LOADING("Loading");
static constexpr ES<8> S_HOOKING("Hooking");
static constexpr ES<5> S_INIT("Init");

inline void EncryptStrings() {
    // Strings encrypted at compile time, decrypt on use
    char loading[8], hooking[8], init[5];
    S_LOADING.decrypt(loading);
    S_HOOKING.decrypt(hooking);
    S_INIT.decrypt(init);
}

inline bool IsDebuggerAttached() {
    char buf[1024];
    int fd = open("/proc/self/status", O_RDONLY);
    if (fd < 0) return false;
    
    int bytesRead = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    
    if (bytesRead <= 0) return false;
    buf[bytesRead] = '\0';
    
    char* tracerPid = strstr(buf, "TracerPid:");
    if (tracerPid) {
        int pid = atoi(tracerPid + 10);
        if (pid != 0) {
            SILENT("Debugger detected");
            return true;
        }
    }
    return false;
}

inline bool CheckAntiDebug() {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        _exit(0);
        return true;
    }
    
    if (IsDebuggerAttached()) {
        _exit(0);
        return true;
    }
    
    FILE* fp = fopen("/proc/self/cmdline", "r");
    if (fp) {
        char cmdline[256];
        fgets(cmdline, sizeof(cmdline), fp);
        fclose(fp);
        
        // Check for debuggers without using suspicious strings
        if (strstr(cmdline, "gdb") || strstr(cmdline, "lldb") || strstr(cmdline, "frida")) {
            _exit(0);
            return true;
        }
    }
    
    return false;
}

inline bool IsEmulator() {
    struct stat st;
    
    // Check emulator files without obvious strings
    const char* files[] = {
        "\x2f\x73\x79\x73\x74\x65\x6d\x2f\x6c\x69\x62\x2f\x6c\x69\x62\x63\x5f\x6d\x61\x6c\x6c\x6f\x63\x5f\x64\x65\x62\x75\x67\x5f\x71\x65\x6d\x75\x2e\x73\x6f",
        "\x2f\x64\x65\x76\x2f\x73\x6f\x63\x6b\x65\x74\x2f\x71\x65\x6d\x75\x64",
        "\x2f\x64\x65\x76\x2f\x71\x65\x6d\x75\x5f\x70\x69\x70\x65",
    };
    
    for (const char* file : files) {
        char path[64];
        // Decrypt XOR 0x55
        for (int i = 0; i < strlen(file); i++) {
            path[i] = file[i] ^ 0x55;
        }
        path[strlen(file)] = '\0';
        
        if (stat(path, &st) == 0) {
            return true;
        }
    }
    
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "goldfish") || strstr(line, "ranchu")) {
                fclose(fp);
                return true;
            }
        }
        fclose(fp);
    }
    
    return false;
}

inline void CheckMemoryIntegrity() {
    static bool initialized = false;
    static uint32_t originalChecksum = 0;
    
    extern char __executable_start;
    extern char etext;
    
    size_t codeSize = &etext - &__executable_start;
    
    uint32_t checksum = 0;
    for (size_t i = 0; i < codeSize; i++) {
        checksum += (uint8_t)(&__executable_start)[i];
    }
    
    if (!initialized) {
        originalChecksum = checksum;
        initialized = true;
    } else if (checksum != originalChecksum) {
        _exit(0);
    }
}

inline void InitProtection() {
    EncryptStrings();
    CheckAntiDebug();
    CheckMemoryIntegrity();
    
    if (IsEmulator()) {
        _exit(0);
    }
}

}
#endif

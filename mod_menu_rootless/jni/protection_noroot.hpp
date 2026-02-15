#ifndef PROTECTION_NOROOT_HPP
#define PROTECTION_NOROOT_HPP

#include <sys/ptrace.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>

#define LOG_TAG "Protection_NoRoot"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

namespace Protection {

static const char* g_EncryptedStrings[] = {
    "\x4c\x6f\x61\x64\x69\x6e\x67",
    "\x48\x6f\x6f\x6b\x69\x6e\x67",
    "\x49\x6e\x69\x74",
};

void XorEncryptDecrypt(char* data, size_t len, uint8_t key) {
    for (size_t i = 0; i < len; i++) {
        data[i] ^= key;
    }
}

void EncryptStrings() {
    for (size_t i = 0; i < sizeof(g_EncryptedStrings) / sizeof(char*); i++) {
        char* str = (char*)g_EncryptedStrings[i];
        XorEncryptDecrypt(str, strlen(str), 0xAB);
    }
}

bool IsDebuggerAttached() {
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
            LOGD("Debugger detected! TracerPid: %d", pid);
            return true;
        }
    }
    
    return false;
}

bool CheckAntiDebug() {
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        LOGD("PTRACE_TRACEME failed - debugger attached");
        exit(0);
        return true;
    }
    
    if (IsDebuggerAttached()) {
        LOGD("Debugger detected via /proc");
        exit(0);
        return true;
    }
    
    FILE* fp = fopen("/proc/self/cmdline", "r");
    if (fp) {
        char cmdline[256];
        fgets(cmdline, sizeof(cmdline), fp);
        fclose(fp);
        
        if (strstr(cmdline, "gdbserver") || strstr(cmdline, "lldb")) {
            LOGD("Debug server detected");
            exit(0);
            return true;
        }
    }
    
    return false;
}

bool IsEmulator() {
    struct stat st;
    
    const char* emulator_files[] = {
        "/system/lib/libc_malloc_debug_qemu.so",
        "/sys/qemu_trace",
        "/system/bin/qemu-props",
        "/dev/socket/qemud",
        "/dev/qemu_pipe",
    };
    
    for (const char* file : emulator_files) {
        if (stat(file, &st) == 0) {
            LOGD("Emulator file detected: %s", file);
            return true;
        }
    }
    
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "goldfish") || strstr(line, "ranchu")) {
                fclose(fp);
                LOGD("Emulator CPU detected");
                return true;
            }
        }
        fclose(fp);
    }
    
    char value[PROP_VALUE_MAX];
    __system_property_get("ro.kernel.qemu", value);
    if (strcmp(value, "1") == 0) {
        LOGD("QEMU property detected");
        return true;
    }
    
    __system_property_get("ro.hardware", value);
    if (strstr(value, "goldfish") || strstr(value, "ranchu")) {
        LOGD("Emulator hardware detected");
        return true;
    }
    
    return false;
}

void CheckMemoryIntegrity() {
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
    } else {
        if (checksum != originalChecksum) {
            LOGD("Memory integrity check failed!");
            exit(0);
        }
    }
}

void InitProtection() {
    LOGD("Initializing protection (no-root)...");
    
    EncryptStrings();
    
    CheckAntiDebug();
    CheckMemoryIntegrity();
    
    if (IsEmulator()) {
        LOGD("Running on emulator - exiting");
        exit(0);
    }
    
    LOGD("Protection initialized");
}

}

#endif

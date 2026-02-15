#ifndef SUBSTRATE_H
#define SUBSTRATE_H

#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __aarch64__
#define INSTRUCTION_SIZE 4
#else
#define INSTRUCTION_SIZE 4
#endif

static void MSHookFunction(void* target, void* replacement, void** original) {
    if (!target || !replacement) return;
    
    uintptr_t pageStart = ((uintptr_t)target) & ~(sysconf(_SC_PAGESIZE) - 1);
    size_t pageLen = sysconf(_SC_PAGESIZE) * 2;
    
    if (mprotect((void*)pageStart, pageLen, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        return;
    }
    
    if (original) {
        *original = target;
    }
    
#ifdef __aarch64__
    uint32_t* instructions = (uint32_t*)target;
    
    uintptr_t offset = ((uintptr_t)replacement - (uintptr_t)target) >> 2;
    
    if ((offset & 0xFC000000) == 0) {
        instructions[0] = 0x14000000 | (offset & 0x03FFFFFF);
    } else {
        instructions[0] = 0x58000051;
        instructions[1] = 0xD61F0220;
        *(uint64_t*)&instructions[2] = (uint64_t)replacement;
    }
#else
    uint32_t* instructions = (uint32_t*)target;
    
    int32_t offset = ((int32_t)replacement - (int32_t)target - 8) >> 2;
    
    if ((offset & 0xFF000000) == 0 || (offset & 0xFF000000) == 0xFF000000) {
        instructions[0] = 0xEA000000 | (offset & 0x00FFFFFF);
    } else {
        instructions[0] = 0xE51FF004;
        instructions[1] = (uint32_t)replacement;
    }
#endif
    
    __builtin___clear_cache((char*)target, (char*)target + 32);
    
    mprotect((void*)pageStart, pageLen, PROT_READ | PROT_EXEC);
}

#ifdef __cplusplus
}
#endif

#endif

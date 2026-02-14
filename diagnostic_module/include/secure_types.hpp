#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace dm {

// Compile-time string encryption using XOR with rotation
// Avoids string literals in binary
namespace obf {
    template<size_t N, uint32_t KEY>
    struct encrypted_string {
        char data[N];
        static constexpr uint32_t key = KEY;
        
        constexpr encrypted_string(const char (&str)[N]) {
            for (size_t i = 0; i < N; ++i) {
                data[i] = str[i] ^ (key + (i * 7));
            }
        }
        
        void decrypt(char* out) const {
            for (size_t i = 0; i < N; ++i) {
                out[i] = data[i] ^ (key + (i * 7));
            }
        }
    };
}

#define DM_ENC(str) \
    ([]() { \
        static constexpr dm::obf::encrypted_string<sizeof(str), __LINE__ * 0x5A827999> enc(str); \
        char decrypted[sizeof(str)]; \
        enc.decrypt(decrypted); \
        return decrypted; \
    }())

// Secure handle type - prevents trivial scanning
using secure_handle_t = uintptr_t;

template<typename T>
inline secure_handle_t make_handle(T* ptr) {
    return reinterpret_cast<secure_handle_t>(ptr) ^ 0xDEADBEEFCAFEBABE;
}

template<typename T>
inline T* from_handle(secure_handle_t handle) {
    return reinterpret_cast<T*>(handle ^ 0xDEADBEEFCAFEBABE);
}

// Memory pointer with relative offset (position-independent)
template<typename T>
struct relative_ptr {
    ptrdiff_t offset;
    
    relative_ptr() = default;
    explicit relative_ptr(uintptr_t base, T* ptr) 
        : offset(reinterpret_cast<uintptr_t>(ptr) - base) {}
    
    T* get(uintptr_t base) const {
        return reinterpret_cast<T*>(base + offset);
    }
    
    void set(uintptr_t base, T* ptr) {
        offset = reinterpret_cast<uintptr_t>(ptr) - base;
    }
};

// Cache-line aligned data to prevent false sharing
struct alignas(64) cache_aligned {
    uint8_t padding[64];
};

// Junk code generator - adds noise to binary
#define DM_JUNK() \
    do { \
        volatile int _junk = __LINE__; \
        _junk = (_junk * 1103515245 + 12345) & 0x7fffffff; \
        (void)_junk; \
    } while(0)

} // namespace dm

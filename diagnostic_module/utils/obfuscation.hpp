#pragma once

#include <cstdint>
#include <array>

namespace diag::obf {

// Compile-time string encryption using XOR with rotation
// Provides basic protection against static string analysis
template<size_t N, uint8_t Key>
struct EncryptedString {
    std::array<char, N> data{};
    static constexpr uint8_t key = Key;
    
    constexpr EncryptedString(const char (&str)[N]) {
        for (size_t i = 0; i < N - 1; ++i) {
            uint8_t rotated = (static_cast<uint8_t>(str[i]) << (i % 8)) | 
                             (static_cast<uint8_t>(str[i]) >> (8 - (i % 8)));
            data[i] = static_cast<char>(rotated ^ (key + i));
        }
        data[N - 1] = '\0';
    }
};

// Runtime decryption
template<size_t N, uint8_t Key>
inline void decrypt_string(char (&out)[N], const EncryptedString<N, Key>& enc) {
    for (size_t i = 0; i < N - 1; ++i) {
        uint8_t decrypted = static_cast<uint8_t>(enc.data[i]) ^ (Key + i);
        uint8_t rotated = (decrypted >> (i % 8)) | (decrypted << (8 - (i % 8)));
        out[i] = static_cast<char>(rotated);
    }
    out[N - 1] = '\0';
}

// Junk code insertion macros for control flow obfuscation
#define OBFUSCATE_FLOW() \
    do { \
        volatile int _junk = __LINE__; \
        _junk = (_junk * 0x5A827999) ^ 0x6ED9EBA1; \
        if (_junk == 0xDEADBEEF) [[unlikely]] { \
            __builtin_unreachable(); \
        } \
    } while(0)

// Opaque predicate - always true but hard to prove statically
inline bool opaque_true() {
    volatile int a = 0x12345678;
    volatile int b = 0x87654321;
    return ((a ^ b) + (a & b) + (~a & b) + (a & ~b)) == (a | b);
}

// Function pointer indirection layer
template<typename Ret, typename... Args>
class IndirectCall {
    using FuncPtr = Ret(*)(Args...);
    FuncPtr target_;
    uintptr_t cookie_;
    
public:
    explicit IndirectCall(FuncPtr target) 
        : target_(target), cookie_(0x9E3779B97F4A7C15ULL) {}
    
    Ret operator()(Args... args) {
        OBFUSCATE_FLOW();
        auto* actual = reinterpret_cast<FuncPtr>(
            reinterpret_cast<uintptr_t>(target_) ^ cookie_
        );
        return actual(args...);
    }
};

// Constant obfuscation - hide immediate values
template<typename T, T Value, uint64_t Seed>
struct ObfuscatedConstant {
    static constexpr T decode() {
        constexpr T obf = Value ^ static_cast<T>(Seed);
        return obf ^ static_cast<T>(Seed);
    }
};

#define OBF_CONST(type, value) \
    (diag::obf::ObfuscatedConstant<type, value, __COUNTER__ * 0x9E3779B9ULL>::decode())

} // namespace diag::obf

// Macro for encrypted string literals
#define ENC_STR(str) ([]() -> const char* { \
    static char buf[sizeof(str)]; \
    static constexpr auto enc = diag::obf::EncryptedString<sizeof(str), __LINE__ % 256>(str); \
    diag::obf::decrypt_string(buf, enc); \
    return buf; \
}())

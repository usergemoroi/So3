#ifndef STRING_ENCRYPT_HPP
#define STRING_ENCRYPT_HPP

#include <cstddef>
#include <cstdint>

// Compile-time string encryption with random key
template<size_t N, uint8_t K1, uint8_t K2, uint8_t K3>
class EncryptedString {
private:
    char data[N];
    static constexpr uint8_t rolling_key(size_t i) {
        return static_cast<uint8_t>((K1 + i * K2) ^ (K3 >> (i % 8)));
    }
    
public:
    template<size_t... I>
    constexpr EncryptedString(const char (&str)[N], std::index_sequence<I...>) 
        : data{static_cast<char>(str[I] ^ rolling_key(I))...} {}
    
    constexpr EncryptedString(const char (&str)[N]) 
        : EncryptedString(str, std::make_index_sequence<N>{}) {}
    
    void decrypt(char* out) const {
        for (size_t i = 0; i < N; i++) {
            out[i] = data[i] ^ rolling_key(i);
        }
    }
    
    static constexpr size_t size() { return N; }
};

// Macro for easy usage
#define ENC_STR(str) ([]() { \
    constexpr auto enc = EncryptedString<sizeof(str), \
        static_cast<uint8_t>(__COUNTER__ * 7 + 0x42), \
        static_cast<uint8_t>(__LINE__ * 13 + 0xAB), \
        static_cast<uint8_t>(sizeof(str) * 17 + 0xEF)>>(str); \
    static char decrypted[sizeof(str)]; \
    enc.decrypt(decrypted); \
    return decrypted; \
}())

// Safe logging that compiles to nothing in release
#ifdef DEBUG_MODE
    #define SAFE_LOG(...)
#else
    #define SAFE_LOG(...) ((void)0)
#endif

#endif

#include "obfuscation.hpp"
#include <cstdint>

namespace diag::obf {

// Runtime string decryption helper
void decrypt_buffer(uint8_t* data, size_t len, uint8_t key) {
    for (size_t i = 0; i < len; ++i) {
        uint8_t rotated = data[i] ^ (key + i);
        data[i] = (rotated >> (i % 8)) | (rotated << (8 - (i % 8)));
    }
}

// Simple XOR encryption for data blobs
void xor_encrypt(uint8_t* data, size_t len, const uint8_t* key, size_t key_len) {
    for (size_t i = 0; i < len; ++i) {
        data[i] ^= key[i % key_len];
    }
}

// Rotate bits left
uint32_t rotate_left(uint32_t value, int bits) {
    return (value << bits) | (value >> (32 - bits));
}

uint32_t rotate_right(uint32_t value, int bits) {
    return (value >> bits) | (value << (32 - bits));
}

} // namespace diag::obf

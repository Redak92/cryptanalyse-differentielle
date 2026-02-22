#pragma once

#include "../../interfaces/ICipher.h"
#include <cstdint>
#include <array>

namespace diffcrypto
{




constexpr uint32_t NUM_FEISTEL_ROUNDS = 32;  

/**
 * CustomFeistel: A 12-bit Feistel cipher for differential cryptanalysis.
 *
 * Architecture:
 *   - Block size: 12 bits (6-bit left half || 6-bit right half)
 *   - Key size: 16 bits
 *   - Feistel rounds: NUM_FEISTEL_ROUNDS (configurable at top of file)
 *   - Final swap applied
 *
 * Round function F(R, K):
 *   Simple 6-bit S-box: SBOX6[(R ^ K) & 0x3F]
 */
class CustomFeistel : public ICipher
{
public:
    explicit CustomFeistel(uint16_t master_key);

    Block encrypt(Block plaintext) const override;
    Block decrypt(Block ciphertext) const override;
    int getBlockSize() const override;

    // CUDA support: expose cipher parameters
    const uint8_t* getRoundKeys() const { return round_keys.data(); }
    static const uint8_t* getSBox() { return SBOX6; }
    static constexpr uint32_t getNumRounds() { return NUM_FEISTEL_ROUNDS; }
    static constexpr uint32_t getBlockBits() { return BLOCK_BITS; }

private:
    static constexpr uint32_t BLOCK_BITS = 12;
    static constexpr uint32_t HALF_BITS = 6;
    static constexpr uint32_t SBOX6_SIZE = 64;

    uint16_t master_key;
    std::array<uint8_t, NUM_FEISTEL_ROUNDS> round_keys;

    // S-box 6 bits -> 6 bits (64 entrées)
    static constexpr uint8_t SBOX6[SBOX6_SIZE] = {
        0x23, 0x15, 0x38, 0x0E, 0x2C, 0x19, 0x07, 0x31,
        0x1A, 0x26, 0x0D, 0x3F, 0x12, 0x2E, 0x05, 0x39,
        0x34, 0x08, 0x1D, 0x2B, 0x16, 0x00, 0x3A, 0x0C,
        0x21, 0x1F, 0x0A, 0x36, 0x03, 0x2D, 0x18, 0x24,
        0x0F, 0x33, 0x1E, 0x28, 0x11, 0x3D, 0x06, 0x2A,
        0x1C, 0x30, 0x09, 0x37, 0x02, 0x1B, 0x3E, 0x14,
        0x29, 0x0B, 0x35, 0x1A, 0x22, 0x04, 0x3C, 0x10,
        0x2F, 0x13, 0x27, 0x01, 0x3B, 0x17, 0x0A, 0x20
    };

    void schedule_key();
    uint8_t round_function(uint8_t right_half, uint8_t round_key) const;
};

} 
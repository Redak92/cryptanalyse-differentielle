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
 *   1. Expansion: 6-bit → 8-bit (with repeated bits)
 *   2. Key mixing: XOR with 8-bit round key
 *   3. Substitution: Split into two 4-bit parts, apply S-boxes
 *   4. Permutation: Permute 8 bits and compress to 6 bits
 */
class CustomFeistel : public ICipher
{
public:
    explicit CustomFeistel(uint16_t master_key);

    Block encrypt(Block plaintext) const override;
    Block decrypt(Block ciphertext) const override;
    int getBlockSize() const override;
    uint32_t block_size_bits() const override;

private:
    static constexpr uint32_t BLOCK_BITS = 12;
    static constexpr uint32_t HALF_BITS = 6;
    static constexpr uint32_t SUBKEY_BITS = 8;
    static constexpr uint32_t SBOX_SIZE = 16;

    uint16_t master_key;
    std::array<uint8_t, NUM_FEISTEL_ROUNDS> round_keys;

    
    static constexpr uint8_t SBOX0[SBOX_SIZE] = {
        0xE, 0x4, 0xD, 0x1, 0x2, 0xF, 0xB, 0x8,
        0x3, 0xA, 0x6, 0xC, 0x5, 0x9, 0x0, 0x7
    };

    static constexpr uint8_t SBOX1[SBOX_SIZE] = {
        0x3, 0xA, 0xE, 0x4, 0x9, 0xF, 0x6, 0x1,
        0xD, 0x0, 0xB, 0x5, 0xC, 0x2, 0x7, 0x8
    };

    
    
    static constexpr uint8_t EXPANSION[8] = {
        5, 4, 3, 2, 1, 0, 5, 4
    };

    
    static constexpr uint8_t PERMUTATION[6] = {
        0, 2, 4, 5, 6, 7
    };

    
    void schedule_key();
    uint8_t round_function(uint8_t right_half, uint8_t round_key) const;
    uint8_t expand(uint8_t input) const;
    uint8_t substitute(uint8_t input) const;
    uint8_t permute_compress(uint8_t input) const;
};

} 
#include "CustomFeistel.h"

namespace diffcrypto
{

CustomFeistel::CustomFeistel(uint16_t master_key)
    : master_key(master_key)
{
    schedule_key();
}

void CustomFeistel::schedule_key()
{
    uint16_t key = master_key;

    for (int i = 0; i < NUM_FEISTEL_ROUNDS; ++i)
    {
        // Rotation modulo 16 pour éviter les décalages invalides (>= 16 bits)
        int shift = (i + 1) % 16;
        uint16_t rotated = (key << shift) | (key >> (16 - shift));
        
        // Clé de tour sur 6 bits pour correspondre à la S-box
        uint8_t round_key = static_cast<uint8_t>((rotated ^ (0xA5 + i * 0x33)) & 0x3F);
        round_keys[i] = round_key;
        key = ((key >> 1) | ((key & 1) << 15)) ^ (0x3C7 + i);
    }
}

uint8_t CustomFeistel::round_function(uint8_t right_half, uint8_t round_key) const
{
    // Simple S-box 6 bits: F(R, K) = SBOX6[(R ^ K) & 0x3F]
    uint8_t input = (right_half ^ round_key) & 0x3F;
    return SBOX6[input];
}

Block CustomFeistel::encrypt(Block plaintext) const
{
    uint16_t block = plaintext & 0xFFF;
    uint8_t L = (block >> HALF_BITS) & 0x3F;
    uint8_t R = block & 0x3F;

    for (int round = 0; round < NUM_FEISTEL_ROUNDS; ++round)
    {
        uint8_t temp = L;
        L = R;
        R = temp ^ round_function(R, round_keys[round]);
    }

    uint8_t tmp = L;
    L = R;
    R = tmp;

    return (static_cast<Block>(L) << HALF_BITS) | R;
}

uint32_t CustomFeistel::block_size_bits() const
{
    return BLOCK_BITS;
}

int CustomFeistel::getBlockSize() const
{
    return static_cast<int>(BLOCK_BITS);
}

Block CustomFeistel::decrypt(Block ciphertext) const
{
    uint16_t block = ciphertext & 0xFFF;
    uint8_t L = (block >> HALF_BITS) & 0x3F;
    uint8_t R = block & 0x3F;

    // Undo final swap
    uint8_t tmp = L;
    L = R;
    R = tmp;

    // Reverse rounds
    for (int round = NUM_FEISTEL_ROUNDS - 1; round >= 0; --round)
    {
        uint8_t temp = R;
        R = L;
        L = temp ^ round_function(L, round_keys[round]);
    }

    return (static_cast<Block>(L) << HALF_BITS) | R;
}

}

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
        uint16_t rotated = (key << (i + 1)) | (key >> (16 - (i + 1)));
        uint8_t round_key = static_cast<uint8_t>((rotated ^ (0xA5 + i * 0x33)) & 0xFF);
        round_keys[i] = round_key;
        key = ((key >> 1) | ((key & 1) << 15)) ^ (0x3C7 + i);
    }
}

uint8_t CustomFeistel::expand(uint8_t input) const
{
    uint8_t output = 0;
    for (int i = 0; i < 8; ++i)
    {
        uint8_t bit_pos = EXPANSION[i];
        uint8_t bit = (input >> bit_pos) & 1;
        output |= (bit << i);
    }
    return output;
}

uint8_t CustomFeistel::substitute(uint8_t input) const
{
    uint8_t left_nibble = (input >> 4) & 0xF;
    uint8_t right_nibble = input & 0xF;

    uint8_t left_out = SBOX0[left_nibble];
    uint8_t right_out = SBOX1[right_nibble];

    return (left_out << 4) | right_out;
}

uint8_t CustomFeistel::permute_compress(uint8_t input) const
{
    uint8_t output = 0;
    for (int i = 0; i < HALF_BITS; ++i)
    {
        uint8_t bit_pos = PERMUTATION[i];
        uint8_t bit = (input >> bit_pos) & 1;
        output |= (bit << i);
    }
    return output;
}

uint8_t CustomFeistel::round_function(uint8_t right_half, uint8_t round_key) const
{
    uint8_t expanded = expand(right_half);
    uint8_t mixed = expanded ^ round_key;
    uint8_t substituted = substitute(mixed);
    return permute_compress(substituted) & 0x3F;
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

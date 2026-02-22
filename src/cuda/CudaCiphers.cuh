#pragma once

#include <cuda_runtime.h>
#include <cstdint>

namespace diffcrypto {
namespace cuda {

__constant__ uint8_t d_SBOX6[64];
__constant__ uint8_t d_round_keys[64];
__constant__ int d_num_rounds;
__constant__ int d_block_bits;

__device__ __forceinline__
uint8_t feistel_round_function(uint8_t right_half, uint8_t round_key)
{
    return d_SBOX6[(right_half ^ round_key) & 0x3F];
}

__device__ __forceinline__
uint64_t feistel_encrypt_device(uint64_t plaintext)
{
    constexpr int HALF_BITS = 6;
    
    uint8_t L = (plaintext >> HALF_BITS) & 0x3F;
    uint8_t R = plaintext & 0x3F;

    for (int round = 0; round < d_num_rounds; ++round)
    {
        uint8_t temp = L;
        L = R;
        R = temp ^ feistel_round_function(R, d_round_keys[round]);
    }

    uint8_t tmp = L;
    L = R;
    R = tmp;

    return (static_cast<uint64_t>(L) << HALF_BITS) | R;
}

__device__ __forceinline__
uint64_t encrypt_device(uint64_t plaintext, int cipher_type = 0)
{
    switch (cipher_type)
    {
        case 0:
            return feistel_encrypt_device(plaintext);
        default:
            return feistel_encrypt_device(plaintext);
    }
}

}
}

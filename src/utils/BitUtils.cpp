//
// Created by alexandre on 30/01/2026.
//

#include "BitUtils.h"

namespace Utils {

uint64_t xor_blocks(uint64_t a, uint64_t b)
{
    return a ^ b;
}

uint64_t make_pair_second(uint64_t x, uint64_t delta)
{
    return x ^ delta;
}

uint64_t compute_output_difference(uint64_t c1, uint64_t c2)
{
    return c1 ^ c2;
}

uint64_t number_of_blocks(uint32_t n_bits)
{
    return 1ULL << n_bits;
}

bool next_block(uint64_t& value, uint32_t n_bits)
{
    uint64_t max_val = (1ULL << n_bits);
    if (value + 1 >= max_val)
    {
        value = 0;
        return false;
    }
    value++;
    return true;
}

double compute_probability(uint64_t count, uint64_t total)
{
    return (total == 0) ? 0.0 : static_cast<double>(count) / static_cast<double>(total);
}

} // Utils

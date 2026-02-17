



#ifndef CRYPTANALYSE_DIFFERENTIELLE_BITUTILS_H
#define CRYPTANALYSE_DIFFERENTIELLE_BITUTILS_H

#include <cstdint>

namespace Utils {
    uint64_t xor_blocks(uint64_t a, uint64_t b);
    uint64_t make_pair_second(uint64_t x, uint64_t delta);
    uint64_t compute_output_difference(uint64_t c1, uint64_t c2);
    uint64_t number_of_blocks(uint32_t n_bits);
    bool next_block(uint64_t& value, uint32_t n_bits);
    double compute_probability(uint64_t count, uint64_t total);
} 

#endif 
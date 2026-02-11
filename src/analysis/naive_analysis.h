#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <utility>
#include "../interfaces/ICipher.h"

namespace diffcrypto
{

// Block est déjà défini dans Types.h (inclus via ICipher.h)
using Count  = uint64_t;
using Prob   = double;

struct DifferentialPair
{
    Block delta_in;
    Block delta_out;
    Prob  probability;
};

struct CipherParams
{
    uint32_t block_size_bits;   
    uint64_t total_plaintexts;  
};

class DifferentialDistributionTable
{
public:
    DifferentialDistributionTable(uint32_t n_bits);

    void reset();
    void increment(Block delta_in, Block delta_out);
    void normalize(uint64_t total_samples);

    Prob  get_probability(Block delta_in, Block delta_out) const;
    Count get_count(Block delta_in, Block delta_out) const;

    DifferentialPair find_best_non_trivial() const;
    std::vector<DifferentialPair> find_best_non_trivial_top_n(size_t num_results) const;

    uint32_t block_bits() const;
    uint64_t table_dimension() const;

private:
    uint32_t n;
    std::vector<Count> counts;      
    std::vector<Prob>  probabilities;

    size_t index(Block dx, Block dy) const;
};

Block xor_blocks(Block a, Block b);

Block make_pair_second(Block x, Block delta);

std::pair<Block, Block> compute_cipher_pair(
    const ICipher& cipher,
    Block x,
    Block delta_in
);

Block compute_output_difference(Block c1, Block c2);

void process_single_pair(
    const ICipher& cipher,
    Block x,
    Block delta_in,
    DifferentialDistributionTable& ddt
);

void process_fixed_input_difference(
    const ICipher& cipher,
    Block delta_in,
    DifferentialDistributionTable& ddt
);

void compute_full_ddt_exhaustive(
    const ICipher& cipher,
    DifferentialDistributionTable& ddt
);

void normalize_ddt(
    DifferentialDistributionTable& ddt,
    uint64_t total_samples_per_delta
);

DifferentialPair find_global_best_differential(
    const DifferentialDistributionTable& ddt
);

std::vector<DifferentialPair> find_global_best_differentials_top_n(
    const DifferentialDistributionTable& ddt,
    size_t n
);

uint64_t number_of_blocks(uint32_t n_bits);

bool next_block(Block& value, uint32_t n_bits);

Prob compute_probability(Count count, uint64_t total);

void compute_all_probabilities(DifferentialDistributionTable& ddt);
void print_ddt_summary(const DifferentialDistributionTable& ddt);
void export_ddt_to_csv(const DifferentialDistributionTable& ddt,
                       const std::string& filename);

DifferentialPair run_exhaustive_differential_analysis(
    const ICipher& cipher
);

} 

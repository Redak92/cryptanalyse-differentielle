#pragma once
#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <utility>
#include "../interfaces/ICipher.h"
#include "../utils/BitUtils.h"

namespace diffcrypto
{

// Maximum iterations per differential search loop (0 = no limit, use full block count)
// Runtime-configurable via get_max_iterations() / set_max_iterations()
inline uint64_t g_max_iterations = 0;

inline uint64_t get_max_iterations() { return g_max_iterations; }
inline void set_max_iterations(uint64_t value) { g_max_iterations = value; }

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

    // CUDA
    void setCounts(const uint64_t* data, size_t size);
    uint64_t* getCountsPtr() { return counts.data(); }
    const uint64_t* getCountsPtr() const { return counts.data(); }

private:
    uint32_t n;
    std::vector<Count> counts;      
    std::vector<Prob>  probabilities;

    size_t index(Block dx, Block dy) const;
};

std::pair<Block, Block> compute_cipher_pair(
    const ICipher& cipher,
    Block x,
    Block delta_in
);

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

void compute_all_probabilities(DifferentialDistributionTable& ddt);

DifferentialPair run_exhaustive_differential_analysis(
    const ICipher& cipher
);

/**
 * @brief Version optimisée mémoire de l'analyse exhaustive
 * 
 * Utilise O(2^n + k) mémoire au lieu de O(2^(2n)) en traitant
 * chaque delta_in séquentiellement avec un compteur temporaire.
 * 
 * @param cipher Le chiffrement à analyser
 * @param top_k Nombre de meilleures différentielles à conserver
 * @return std::vector<DifferentialPair> Les top_k meilleures différentielles
 */
std::vector<DifferentialPair> run_exhaustive_differential_analysis_streaming(
    const ICipher& cipher,
    size_t top_k = 1
);

}
 

#include "naive_analysis.h"
#include "../utils/BitUtils.h"
#include "../utils/DDTExport.h"
#include <algorithm>
#include <iostream>
#include <fstream>

namespace diffcrypto
{

// ============================================================================
// DifferentialDistributionTable Implementation
// ============================================================================

DifferentialDistributionTable::DifferentialDistributionTable(uint32_t n_bits)
    : n(n_bits)
{
    uint64_t table_size = static_cast<uint64_t>(1) << (2 * n);
    counts.resize(table_size, 0);
    probabilities.resize(table_size, 0.0);
}

void DifferentialDistributionTable::reset()
{
    std::fill(counts.begin(), counts.end(), 0);
    std::fill(probabilities.begin(), probabilities.end(), 0.0);
}

size_t DifferentialDistributionTable::index(Block dx, Block dy) const
{   
    uint64_t mask = (1ULL << n) - 1;
    return static_cast<size_t>(((dx & mask) << n) | (dy & mask));
}

void DifferentialDistributionTable::increment(Block delta_in, Block delta_out)
{
    counts[index(delta_in, delta_out)]++;
}

void DifferentialDistributionTable::normalize(uint64_t total_samples)
{
    for (size_t i = 0; i < probabilities.size(); ++i)
    {
        probabilities[i] = static_cast<Prob>(counts[i]) / static_cast<Prob>(total_samples);
    }
}

Prob DifferentialDistributionTable::get_probability(Block delta_in, Block delta_out) const
{
    return probabilities[index(delta_in, delta_out)];
}

Count DifferentialDistributionTable::get_count(Block delta_in, Block delta_out) const
{
    return counts[index(delta_in, delta_out)];
}

void DifferentialDistributionTable::setCounts(const uint64_t* data, size_t size)
{
    if (size <= counts.size())
    {
        std::copy(data, data + size, counts.begin());
    }
}

DifferentialPair DifferentialDistributionTable::find_best_non_trivial() const
{
    DifferentialPair best = {0, 0, 0.0};

    for (uint64_t dx = 1; dx < (1ULL << n); ++dx)
    {
        for (uint64_t dy = 0; dy < (1ULL << n); ++dy)
        {
            Prob prob = get_probability(dx, dy);
            if (prob > best.probability)
            {
                best.delta_in = dx;
                best.delta_out = dy;
                best.probability = prob;
            }
        }
    }

    return best;
}

std::vector<DifferentialPair> DifferentialDistributionTable::find_best_non_trivial_top_n(size_t num_results) const
{
    // Collect all non-trivial differentials with their probabilities
    std::vector<std::pair<Prob, DifferentialPair>> all_differentials;

    for (uint64_t dx = 1; dx < (1ULL << n); ++dx)
    {
        for (uint64_t dy = 0; dy < (1ULL << n); ++dy)
        {
            Prob prob = get_probability(dx, dy);
            if (prob > 0.0)  // Only keep non-zero entries
            {
                DifferentialPair pair = {dx, dy, prob};
                all_differentials.push_back({prob, pair});
            }
        }
    }

    // Sort by probability in descending order
    std::sort(all_differentials.begin(), all_differentials.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Return top num_results
    std::vector<DifferentialPair> result;
    size_t count = std::min(num_results, all_differentials.size());
    for (size_t i = 0; i < count; ++i)
    {
        result.push_back(all_differentials[i].second);
    }

    return result;
}

uint32_t DifferentialDistributionTable::block_bits() const
{
    return n;
}

uint64_t DifferentialDistributionTable::table_dimension() const
{
    return 1ULL << n;
}



std::pair<Block, Block> compute_cipher_pair(
    const ICipher& cipher,
    Block x,
    Block delta_in)
{
    return std::make_pair(cipher.encrypt(x), cipher.encrypt(x ^ delta_in));
}




void process_single_pair(
    const ICipher& cipher,
    Block x,
    Block delta_in,
    DifferentialDistributionTable& ddt)
{
    Block c1 = cipher.encrypt(x);
    Block c2 = cipher.encrypt(x ^ delta_in);
    ddt.increment(delta_in, c1 ^ c2);
}

void process_fixed_input_difference(
    const ICipher& cipher,
    Block delta_in,
    DifferentialDistributionTable& ddt)
{
    uint64_t block_count = ddt.table_dimension();
    for (uint64_t x = 0; x < block_count; ++x)
    {
        process_single_pair(cipher, x, delta_in, ddt);
    }
}

void compute_full_ddt_exhaustive(
    const ICipher& cipher,
    DifferentialDistributionTable& ddt)
{
    uint64_t block_count = ddt.table_dimension();
    for (uint64_t delta_in = 1; delta_in < block_count; ++delta_in)
    {
        process_fixed_input_difference(cipher, delta_in, ddt);
    }
}





void normalize_ddt(
    DifferentialDistributionTable& ddt,
    uint64_t total_samples_per_delta)
{
    ddt.normalize(total_samples_per_delta);
}

DifferentialPair find_global_best_differential(
    const DifferentialDistributionTable& ddt)
{
    return ddt.find_best_non_trivial();
}

std::vector<DifferentialPair> find_global_best_differentials_top_n(
    const DifferentialDistributionTable& ddt,
    size_t n)
{
    return ddt.find_best_non_trivial_top_n(n);
}




void compute_all_probabilities(DifferentialDistributionTable& ddt)
{
    ddt.normalize(ddt.table_dimension());
}




DifferentialPair run_exhaustive_differential_analysis(
    const ICipher& cipher)
{
    uint32_t n_bits = cipher.getBlockSize();
    DifferentialDistributionTable ddt(n_bits);

    compute_full_ddt_exhaustive(cipher, ddt);
    normalize_ddt(ddt, 1ULL << n_bits);

    return find_global_best_differential(ddt);
}

}
 

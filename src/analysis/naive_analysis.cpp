#include "naive_analysis.h"
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



Block xor_blocks(Block a, Block b)
{
    return a ^ b;
}

Block make_pair_second(Block x, Block delta)
{
    return x ^ delta;
}

std::pair<Block, Block> compute_cipher_pair(
    const BlockCipher& cipher,
    Block x,
    Block delta_in)
{
    return std::make_pair(cipher.encrypt(x), cipher.encrypt(x ^ delta_in));
}

Block compute_output_difference(Block c1, Block c2)
{
    return c1 ^ c2;
}




void process_single_pair(
    const BlockCipher& cipher,
    Block x,
    Block delta_in,
    DifferentialDistributionTable& ddt)
{
    Block c1 = cipher.encrypt(x);
    Block c2 = cipher.encrypt(x ^ delta_in);
    ddt.increment(delta_in, c1 ^ c2);
}

void process_fixed_input_difference(
    const BlockCipher& cipher,
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
    const BlockCipher& cipher,
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





uint64_t number_of_blocks(uint32_t n_bits)
{
    return 1ULL << n_bits;
}

bool next_block(Block& value, uint32_t n_bits)
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

Prob compute_probability(Count count, uint64_t total)
{
    return (total == 0) ? 0.0 : static_cast<Prob>(count) / static_cast<Prob>(total);
}

void compute_all_probabilities(DifferentialDistributionTable& ddt)
{
    ddt.normalize(ddt.table_dimension());
}





void print_ddt_summary(const DifferentialDistributionTable& ddt)
{
    DifferentialPair best = ddt.find_best_non_trivial();
    std::cout << "DDT Summary (Block size: " << ddt.block_bits() << " bits)\n";
    std::cout << "Best non-trivial differential:\n";
    std::cout << "  ΔX = 0x" << std::hex << best.delta_in << std::dec << "\n";
    std::cout << "  ΔY = 0x" << std::hex << best.delta_out << std::dec << "\n";
    std::cout << "  Probability = " << best.probability << "\n";
}

void export_ddt_to_csv(const DifferentialDistributionTable& ddt,
                       const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error: could not open " << filename << "\n";
        return;
    }

    file << "delta_in,delta_out,count,probability\n";
    uint64_t dim = ddt.table_dimension();
    for (uint64_t dx = 0; dx < dim; ++dx)
    {
        for (uint64_t dy = 0; dy < dim; ++dy)
        {
            Count cnt = ddt.get_count(dx, dy);
            if (cnt > 0)
            {
                file << dx << "," << dy << "," << cnt << "," 
                     << ddt.get_probability(dx, dy) << "\n";
            }
        }
    }
    file.close();
}



DifferentialPair run_exhaustive_differential_analysis(
    const BlockCipher& cipher)
{
    uint32_t n_bits = cipher.block_size_bits();
    DifferentialDistributionTable ddt(n_bits);

    compute_full_ddt_exhaustive(cipher, ddt);
    normalize_ddt(ddt, 1ULL << n_bits);

    return find_global_best_differential(ddt);
}

} 

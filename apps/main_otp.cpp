#include "../src/analysis/naive_analysis.h"
#include "../src/ciphers/One-Time-Pad/otp.h"
#include <iostream>
#include <iomanip>



using namespace diffcrypto;


int main()
{
    OTP cipher(0x3F4A);
    auto best = run_exhaustive_differential_analysis(cipher);

    std::cout << "\n====================================================\n";
    std::cout << "  Results\n";
    std::cout << "====================================================\n\n";

    std::cout << "Best non-trivial differential:\n";
    std::cout << "  ΔX = 0x" << std::hex << std::setw(3) << std::setfill('0') 
              << best.delta_in << std::dec << "\n";
    std::cout << "  ΔY = 0x" << std::hex << std::setw(3) << std::setfill('0') 
              << best.delta_out << std::dec << "\n";
    std::cout << "  Probability = " << std::fixed << std::setprecision(6) 
              << best.probability << "\n";
    std::cout << "  Bias = " << (best.probability - 0.5) << "\n";

    std::cout << "\n====================================================\n";
    
    // Demonstrate the new top-N method
    std::cout << "  Top 5 Non-Trivial Differentials\n";
    std::cout << "====================================================\n\n";
    
    // Re-run analysis to get the DDT for querying top-N
    uint32_t n_bits = static_cast<uint32_t>(cipher.getBlockSize());
    DifferentialDistributionTable ddt(n_bits);
    compute_full_ddt_exhaustive(cipher, ddt);
    normalize_ddt(ddt, 1ULL << n_bits);
    
    auto top5 = ddt.find_best_non_trivial_top_n(5);
    
    for (size_t i = 0; i < top5.size(); ++i)
    {
        const auto& diff = top5[i];
        std::cout << "  #" << (i + 1) << ": ΔX=0x" << std::hex << std::setw(3) 
                  << std::setfill('0') << diff.delta_in << " -> ΔY=0x" 
                  << std::setw(3) << diff.delta_out << std::dec << " (prob=" 
                  << std::fixed << std::setprecision(6) << diff.probability << ")\n";
    }

    std::cout << "\n====================================================\n";
    return 0;
}


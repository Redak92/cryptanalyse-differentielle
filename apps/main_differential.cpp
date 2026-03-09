#include "../src/analysis/naive_analysis.h"
#include "../src/ciphers/CustomFeistel/CustomFeistel.h"
#include <iostream>
#include <iomanip>

using namespace diffcrypto;

int main()
{
    std::cout << "====================================================\n";
    std::cout << "  Exhaustive Differential Cryptanalysis\n";
    std::cout << "  Toy Feistel Cipher (12-bit, 4 rounds)\n";
    std::cout << "====================================================\n\n";

    CustomFeistel cipher(0x5A3C);

    std::cout << "Cipher parameters:\n";
    std::cout << "  Block size: " << cipher.getBlockSize() << " bits\n";
    std::cout << "  Key: 0x5A3C (16 bits)\n";
    std::cout << "  Rounds: " << diffcrypto::NUM_FEISTEL_ROUNDS << " (configurable in CustomFeistel.h)\n";
    std::cout << "  Plaintexts to test: " << (1ULL << cipher.getBlockSize()) << "\n";
    std::cout << "  Complexity: 2^24 ≈ 16 million encryptions\n\n";

    std::cout << "Running exhaustive analysis (memory-optimized)...\n";
    
    // Une seule passe pour obtenir le top 5 (utilise O(2^n) mémoire au lieu de O(2^(2n)))
    auto top5 = run_exhaustive_differential_analysis_streaming(cipher, 5);
    
    if (top5.empty())
    {
        std::cout << "No differentials found.\n";
        return 1;
    }
    
    const auto& best = top5[0];

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
    
    // Top 5 (déjà calculé)
    std::cout << "  Top 5 Non-Trivial Differentials\n";
    std::cout << "====================================================\n\n";
    
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

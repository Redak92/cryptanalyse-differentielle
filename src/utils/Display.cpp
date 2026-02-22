#include "Display.h"

namespace diffcrypto {

void printSeparator()
{
    std::cout << std::string(60, '=') << "\n";
}

void printHeader(const char* title)
{
    printSeparator();
    std::cout << "  " << title << "\n";
    printSeparator();
}

void printDifferentialPair(const DifferentialPair& pair)
{
    std::cout << "(0x" << std::hex << pair.delta_in 
              << " -> 0x" << pair.delta_out << std::dec 
              << ") p=" << pair.probability << "\n";
}

void printDDTSummary(const DifferentialDistributionTable& ddt)
{
    DifferentialPair best = ddt.find_best_non_trivial();
    std::cout << "DDT Summary (Block size: " << ddt.block_bits() << " bits)\n";
    std::cout << "Best non-trivial differential:\n";
    std::cout << "  DX = 0x" << std::hex << best.delta_in << std::dec << "\n";
    std::cout << "  DY = 0x" << std::hex << best.delta_out << std::dec << "\n";
    std::cout << "  Probability = " << best.probability << "\n";
}

}

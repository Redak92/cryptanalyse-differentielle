#ifndef UTILS_H
#define UTILS_H

#include "types.h"
#include <string>
#include <iomanip>
#include <sstream>
#include <bitset>

#ifdef _MSC_VER
    #include <intrin.h>
    #define POPCOUNT __popcnt
#else
    #define POPCOUNT __builtin_popcount
#endif

namespace Utils {

inline std::string toHex(Block value, size_t width = 8) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::setfill('0') << std::setw(width) << value;
    return oss.str();
}

inline std::string differentialToString(const DifferentialPair& diff) {
    return "(" + toHex(diff.first) + " -> " + toHex(diff.second) + ")";
}

inline bool getBit(Block value, int i) {
    return (value >> i) & 1;
}

inline int popcount(Block value) {
    return POPCOUNT(value);
}

inline Block xorBlocks(Block a, Block b) {
    return a ^ b;
}

}

#endif

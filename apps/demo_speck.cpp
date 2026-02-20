//
// Created by alexandre on 30/01/2026.
//

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

#include "../src/ciphers/Speck/Speck.h"
#include "analysis/DifferentialSearch.h"

int main() {
    constexpr int test_rounds = 3;
    const double target_probability = std::pow(2.0, -7.0);

    const Speck speck(0x1918111009080100, test_rounds);
    const DifferentialSearch search(speck);
    std::vector<DifferentialCandidate> results = search.runFundamentalAlgorithm(target_probability);

    std::cout << "Resultats : " << results.size() << " trouves." << std::endl;

    return 0;
}

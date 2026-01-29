#include "differential_search.h"
#include "../utils/utils.h"
#include <iostream>
#include <iomanip>
#include <cmath>

DifferentialSearch::DifferentialSearch(ToyCipher& cipher, uint64_t maxSamples)
    : cipher(cipher), maxSamples(maxSamples) {
    DistinguishedPoints::Config dpConfig;
    dpConfig.numThreads = std::thread::hardware_concurrency();
    dpConfig.maxMarches = maxSamples;
    distinguishedPoints = std::make_unique<DistinguishedPoints>(cipher, dpConfig);
}

void DifferentialSearch::printStatistics() const {
    std::cout << "\n=== Statistiques Points Distingués ===\n";
    std::cout << "Total de marches effectuées : " << distinguishedPoints->getStats().totalWalks << "\n";
    std::cout << "Points distingués trouvés : " << distinguishedPoints->getStats().distinguishedPointsFound << "\n";
    std::cout << "Collisions détectées : " << distinguishedPoints->getStats().collisionsFound << "\n";
    
    double expectedComplexity = std::sqrt(static_cast<double>(1ULL << 32));
    std::cout << "\nComplexité théorique (2^n/2) : " << static_cast<uint64_t>(expectedComplexity) << "\n";
    std::cout << "Progression : " << std::fixed << std::setprecision(2)
              << (100.0 * distinguishedPoints->getStats().totalWalks / maxSamples) << "%\n";
}

std::vector<DistinguishedPoints::CollisionResult>
DifferentialSearch::findCollisionsWithDistinguishedPoints(
    const DistinguishedPoints::Config& config) {
    distinguishedPoints = std::make_unique<DistinguishedPoints>(cipher, config);

    return distinguishedPoints->findCollisions();
}


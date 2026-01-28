#include "differential_search.h"
#include "../utils/utils.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cstring>

DifferentialSearch::DifferentialSearch(ToyCipher& cipher, uint64_t maxSamples)
    : cipher(cipher), maxSamples(maxSamples) {
    
    // Initialiser les optimisations
    ParallelSearch::Config psConfig;
    psConfig.numThreads = std::thread::hardware_concurrency();
    psConfig.samplesPerThread = maxSamples / psConfig.numThreads;
    parallelSearch = std::make_unique<ParallelSearch>(cipher, psConfig);
    
    DistinguishedPoints::Config dpConfig;
    dpConfig.numThreads = std::thread::hardware_concurrency();
    dpConfig.maxMarches = maxSamples;
    distinguishedPoints = std::make_unique<DistinguishedPoints>(cipher, dpConfig);
}

Block DifferentialSearch::computeDerivative(Block x, Difference deltaIn) {
    
    
    
    Block x1 = x;
    Block x2 = x ^ deltaIn;
    
    Block y1 = cipher.encrypt(x1);
    Block y2 = cipher.encrypt(x2);
    
    
    
    Block derivative = y1 ^ y2;
    derivative = (derivative << 13) | (derivative >> 19);  
    
    return derivative;
}

DifferentialCount DifferentialSearch::searchDifferentials(Difference deltaIn) {
    DifferentialCount localDifferentials;
    
    if (deltaIn == 0) {
        return localDifferentials;  
    }
    
    
    CollisionTable collisionTable;
    
    
    for (uint64_t i = 0; i < maxSamples; i++) {
        Block x = static_cast<Block>(i ^ (0x12345678 ^ (i * 0x9E3779B9)));
        
        Block x1 = x;
        Block x2 = x ^ deltaIn;
        
        Block y1 = cipher.encrypt(x1);
        Block y2 = cipher.encrypt(x2);
        
        Difference deltaOut = y1 ^ y2;
        
        
        DifferentialPair diff = {deltaIn, deltaOut};
        localDifferentials[diff]++;
        
        
        globalDifferentials[diff]++;
    }
    
    return localDifferentials;
}

std::vector<std::pair<DifferentialPair, double>>
DifferentialSearch::findBestDifferentials(int topK, float probabilityThreshold) {
    std::vector<std::pair<DifferentialPair, double>> results;
    
    
    for (const auto& entry : globalDifferentials) {
        double probability = static_cast<double>(entry.second) / maxSamples;
        
        if (probability >= probabilityThreshold) {
            results.push_back({entry.first, probability});
        }
    }
    
    
    std::sort(results.begin(), results.end(),
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        });
    
    
    if (results.size() > static_cast<size_t>(topK)) {
        results.resize(topK);
    }
    
    return results;
}

void DifferentialSearch::analyzeMultipleDifferences(const std::vector<Difference>& deltaIns) {
    std::cout << "Analyzing " << deltaIns.size() << " input differences...\n";
    
    for (size_t i = 0; i < deltaIns.size(); i++) {
        std::cout << "  Processing " << (i + 1) << "/" << deltaIns.size() 
                  << " (deltaIn = " << Utils::toHex(deltaIns[i]) << ")...\n";
        searchDifferentials(deltaIns[i]);
    }
}

void DifferentialSearch::printStatistics() const {
    std::cout << "\n=== Differential Statistics ===\n";
    std::cout << "Total differentials found : " 
              << globalDifferentials.size() << "\n";
    
    
    uint64_t maxCount = 0;
    DifferentialPair bestDiff = {0, 0};
    
    for (const auto& entry : globalDifferentials) {
        if (entry.second > maxCount) {
            maxCount = entry.second;
            bestDiff = entry.first;
        }
    }
    
    if (maxCount > 0) {
        double probability = static_cast<double>(maxCount) / maxSamples;
        std::cout << "\nBest Differential:\n";
        std::cout << "  Input  : " << Utils::toHex(bestDiff.first) << "\n";
        std::cout << "  Output : " << Utils::toHex(bestDiff.second) << "\n";
        std::cout << "  Count  : " << maxCount << " / " << maxSamples << "\n";
        std::cout << "  Prob   : " << std::fixed << std::setprecision(6) 
                  << probability << "\n";
    }
}

DifferentialCount DifferentialSearch::searchDifferentialsParallel(
    Difference deltaIn,
    uint32_t numThreads) {
    
    if (!parallelSearch) {
        std::cerr << "Erreur : ParallelSearch non initialisé\n";
        return globalDifferentials;
    }

    ParallelSearch::Config config;
    config.numThreads = numThreads;
    config.samplesPerThread = maxSamples / numThreads;
    config.targetDeltaIn = deltaIn;

    ParallelSearch ps(cipher, config);
    return ps.searchDifferentialsParallel(deltaIn);
}

std::vector<DistinguishedPoints::CollisionResult> 
DifferentialSearch::findCollisionsWithDistinguishedPoints(
    const DistinguishedPoints::Config& config) {
    
    if (!distinguishedPoints) {
        std::cerr << "Erreur : DistinguishedPoints non initialisé\n";
        return {};
    }

    DistinguishedPoints dp(cipher, config);
    return dp.findCollisions();
}


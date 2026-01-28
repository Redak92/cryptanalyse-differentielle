#ifndef DIFFERENTIAL_SEARCH_H
#define DIFFERENTIAL_SEARCH_H

#include "../utils/types.h"
#include "../cipher/toy_cipher.h"
#include "parallel_search.h"
#include "distinguished_points.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <thread>



class DifferentialSearch {
public:
    DifferentialSearch(ToyCipher& cipher, uint64_t maxSamples = 1000000);
    
    
    
    DifferentialCount searchDifferentials(Difference deltaIn);
    
    
    std::vector<std::pair<DifferentialPair, double>> 
    findBestDifferentials(int topK = 10, float probabilityThreshold = 0.001);
    
    
    void analyzeMultipleDifferences(const std::vector<Difference>& deltaIns);
    
    
    void printStatistics() const;

    /**
     * Optimisations
     */
    
    DifferentialCount searchDifferentialsParallel(
        Difference deltaIn,
        uint32_t numThreads = std::thread::hardware_concurrency());
    
    
    std::vector<DistinguishedPoints::CollisionResult> findCollisionsWithDistinguishedPoints(
        const DistinguishedPoints::Config& config);
    
private:
    ToyCipher& cipher;
    uint64_t maxSamples;
    DifferentialCount globalDifferentials;
    
    // Instances d'optimisation
    std::unique_ptr<ParallelSearch> parallelSearch;
    std::unique_ptr<DistinguishedPoints> distinguishedPoints;
    
    
    struct CollisionTable {
        std::unordered_map<Block, std::vector<Block>> table;
        
        void insert(Block hash, Block value) {
            table[hash].push_back(value);
        }
        
        std::vector<Block> findCollisions(Block hash) const {
            auto it = table.find(hash);
            if (it != table.end()) {
                return it->second;
            }
            return {};
        }
    };
    
    
    
    Block computeDerivative(Block x, Difference deltaIn);
};

#endif 

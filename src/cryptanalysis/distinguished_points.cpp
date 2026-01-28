#include "distinguished_points.h"
#include "../utils/utils.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>

DistinguishedPoints::DistinguishedPoints(ToyCipher& cipher, const Config& config)
    : cipher(cipher), config(config) {}

bool DistinguishedPoints::isDistinguished(Block value) const {
    
    uint32_t mask = (1U << config.distinguishedBitCount) - 1;
    return (value & mask) == 0;
}

Block DistinguishedPoints::functionG(Block x) const {
    
    
    Block encrypted = cipher.encrypt(x);
    Block rotated = (x << 5) | (x >> 27);
    return encrypted ^ rotated;
}

DistinguishedPoints::DistinguishedPoint DistinguishedPoints::performWalk(Block startingPoint) {
    Block current = startingPoint;
    uint32_t steps = 0;

    
    while (!isDistinguished(current) && steps < config.maxWalkSteps) {
        current = functionG(current);
        steps++;
    }

    DistinguishedPoint result;
    result.value = current;
    result.startingPoint = startingPoint;
    result.walkLength = steps;

    return result;
}

DistinguishedPoints::CollisionResult DistinguishedPoints::traceback(
    Block x1, Block y1,
    const DistinguishedPoint& x1Point,
    const DistinguishedPoint& y1Point) {
    
    CollisionResult result;
    result.found = false;

    
    Block x_current = x1;
    Block y_current = y1;

    
    for (uint32_t i = 0; i < x1Point.walkLength; i++) {
        x_current = functionG(x_current);
    }
    for (uint32_t i = 0; i < y1Point.walkLength; i++) {
        y_current = functionG(y_current);
    }

    
    if (x_current == y_current) {
        
        Block collision = x_current;
        
        
        Block x_before_dp = x1Point.startingPoint;
        Block y_before_dp = y1Point.startingPoint;
        Block encrypted_x = cipher.encrypt(x_before_dp);
        Block encrypted_y = cipher.encrypt(y_before_dp);

        result.x = x_before_dp;
        result.y = y_before_dp;
        result.collision = collision;
        result.deltaIn = x_before_dp ^ y_before_dp;
        result.deltaOut = encrypted_x ^ encrypted_y;
        result.found = true;
    }

    return result;
}

void DistinguishedPoints::workerThread(uint32_t threadId, uint64_t marcheStart, uint64_t marcheEnd) {
    std::vector<CollisionResult> localCollisions;

    for (uint64_t marche = marcheStart; marche < marcheEnd; marche++) {
        
        Block startPoint = static_cast<Block>(marche ^ (0xAAAAAAAA ^ (marche * 0x9E3779B9)));

        
        DistinguishedPoint dpResult = performWalk(startPoint);

        
        if (!isDistinguished(dpResult.value)) {
            continue;  
        }

        
        {
            std::lock_guard<std::mutex> lock(tableWriteMutex);

            auto it = distinguishedTable.find(dpResult.value);
            if (it != distinguishedTable.end()) {
                
                DistinguishedPoint& existing = it->second;
                
                
                Block x1 = startPoint;
                Block y1 = existing.startingPoint;

                Block encrypted_x1 = cipher.encrypt(x1);
                Block encrypted_y1 = cipher.encrypt(y1);

                if (encrypted_x1 != encrypted_y1) {
                    CollisionResult collision;
                    collision.x = x1;
                    collision.y = y1;
                    collision.collision = dpResult.value;
                    collision.deltaIn = x1 ^ y1;
                    collision.deltaOut = encrypted_x1 ^ encrypted_y1;
                    collision.found = true;
                    localCollisions.push_back(collision);

                    {
                        std::lock_guard<std::mutex> statsLock(stats.statsMutex);
                        stats.collisionsFound++;
                    }
                }
            } else {
                
                distinguishedTable[dpResult.value] = dpResult;
                {
                    std::lock_guard<std::mutex> statsLock(stats.statsMutex);
                    stats.distinguishedPointsFound++;
                }
            }
        }

        {
            std::lock_guard<std::mutex> statsLock(stats.statsMutex);
            stats.totalWalks++;
        }
    }
}

std::vector<DistinguishedPoints::CollisionResult> DistinguishedPoints::findCollisions() {
    std::vector<CollisionResult> allCollisions;

    std::cout << "\n=== Recherche de collisions par Points Distingués ===\n";
    std::cout << "Configuration :\n";
    std::cout << "  Bits distingués : " << config.distinguishedBitCount << "\n";
    std::cout << "  Marches max par thread : " << (config.maxMarches / config.numThreads) << "\n";
    std::cout << "  Nombre de threads : " << config.numThreads << "\n";
    std::cout << "  Itérations max par marche : " << config.maxWalkSteps << "\n";

    
    uint64_t marchesParThread = config.maxMarches / config.numThreads;
    std::vector<std::thread> workers;

    
    for (uint32_t i = 0; i < config.numThreads; i++) {
        uint64_t marcheStart = i * marchesParThread;
        uint64_t marcheEnd = (i == config.numThreads - 1) ? config.maxMarches : (i + 1) * marchesParThread;

        workers.emplace_back(
            &DistinguishedPoints::workerThread,
            this,
            i,
            marcheStart,
            marcheEnd
        );
    }

    
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    
    for (const auto& entry : distinguishedTable) {
        if (allCollisions.size() < 100) {  
            
        }
    }

    return allCollisions;
}

void DistinguishedPoints::printStatistics() const {
    std::cout << "\n=== Statistiques Points Distingués ===\n";
    std::cout << "Total de marches effectuées : " << stats.totalWalks << "\n";
    std::cout << "Points distingués trouvés : " << stats.distinguishedPointsFound << "\n";
    std::cout << "Collisions détectées : " << stats.collisionsFound << "\n";
    
    double expectedComplexity = std::sqrt(static_cast<double>(1ULL << 32));
    std::cout << "\nComplexité théorique (2^n/2) : " << static_cast<uint64_t>(expectedComplexity) << "\n";
    std::cout << "Progression : " << std::fixed << std::setprecision(2)
              << (100.0 * stats.totalWalks / config.maxMarches) << "%\n";
}

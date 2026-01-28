#include "distinguished_points.h"
#include "../utils/utils.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>

DistinguishedPoints::DistinguishedPoints(ToyCipher& cipher, const Config& config)
    : cipher(cipher), config(config) {}

bool DistinguishedPoints::isDistinguished(Block value) const {
    // Un point est distingué si les k bits de poids faible sont à zéro
    uint32_t mask = (1U << config.distinguishedBitCount) - 1;
    return (value & mask) == 0;
}

Block DistinguishedPoints::functionG(Block x) const {
    // G(x) = encrypt(x) XOR (x << 5)
    // Fonction pseudo-aléatoire pour la marche
    Block encrypted = cipher.encrypt(x);
    Block rotated = (x << 5) | (x >> 27);
    return encrypted ^ rotated;
}

DistinguishedPoints::DistinguishedPoint DistinguishedPoints::performWalk(Block startingPoint) {
    Block current = startingPoint;
    uint32_t steps = 0;

    // Marche jusqu'à trouver un point distingué ou atteindre maxWalkSteps
    while (!isDistinguished(current) && steps < config.maxWalkSteps) {
        current = functionG(current);
        steps++;
    }

    DistinguishedPoint result;
    result.value = current;
    result.startingPoint = startingPoint;
    result.walkLength = steps;
    result.threadId = std::this_thread::get_id();

    return result;
}

DistinguishedPoints::CollisionResult DistinguishedPoints::traceback(
    Block x1, Block y1,
    const DistinguishedPoint& x1Point,
    const DistinguishedPoint& y1Point) {
    
    CollisionResult result;
    result.found = false;

    // Remonter à la collision en réexécutant les marches
    Block x_current = x1;
    Block y_current = y1;

    // Remonter jusqu'au point distingué
    for (uint32_t i = 0; i < x1Point.walkLength; i++) {
        x_current = functionG(x_current);
    }
    for (uint32_t i = 0; i < y1Point.walkLength; i++) {
        y_current = functionG(y_current);
    }

    // À ce stade, G^k(x1) = G^m(y1) (points distingués)
    if (x_current == y_current) {
        // Collision trouvée !
        Block collision = x_current;
        
        // Calculer les différences
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
        // Générer un point de départ pseudo-aléatoire
        Block startPoint = static_cast<Block>(marche ^ (0xAAAAAAAA ^ (marche * 0x9E3779B9)));

        // Effectuer une marche
        DistinguishedPoint dpResult = performWalk(startPoint);

        // Vérifier si c'est vraiment distingué
        if (!isDistinguished(dpResult.value)) {
            continue;  // Marche ne s'est pas arrêtée à un point distingué
        }

        // Accès thread-safe à la table
        {
            std::lock_guard<std::mutex> lock(tableWriteMutex);

            auto it = distinguishedTable.find(dpResult.value);
            if (it != distinguishedTable.end()) {
                // COLLISION TROUVÉE !
                DistinguishedPoint& existing = it->second;
                
                // Tracer jusqu'à la collision exacte
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
                // Nouveau point distingué, l'enregistrer
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

    // Diviser le travail entre les threads
    uint64_t marchesParThread = config.maxMarches / config.numThreads;
    std::vector<std::thread> workers;

    // Créer et lancer les worker threads
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

    // Attendre que tous les threads se terminent
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    // Extraire les collisions
    for (const auto& entry : distinguishedTable) {
        if (allCollisions.size() < 100) {  // Limiter à 100 collisions pour éviter la surcharge
            // Les collisions ont déjà été trouvées lors de la construction
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

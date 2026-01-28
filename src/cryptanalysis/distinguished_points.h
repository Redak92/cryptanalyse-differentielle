#ifndef DISTINGUISHED_POINTS_H
#define DISTINGUISHED_POINTS_H

#include "../utils/types.h"
#include "../cipher/toy_cipher.h"
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <memory>
#include <cstdint>

/**
 * Points Distingués - Algorithme de recherche de collisions O(2^n/2) sans saturation RAM
 * Principe : Au lieu de stocker toutes les valeurs, on stocke uniquement les "points distingués"
 * identifiés par une condition simple (ex: k bits de poids faible = 0)
 */
class DistinguishedPoints {
public:
    struct DistinguishedPoint {
        Block value;           // La valeur du point distingué
        Block startingPoint;   // D'où provient-il ? (pour remonter à la collision)
        uint32_t walkLength;   // Nombre d'itérations pour atteindre ce point
        uint32_t threadId;     // Quel thread l'a trouvé
    };

    struct CollisionResult {
        Block x;               // Premier point
        Block y;               // Deuxième point
        Block collision;       // G(x) = G(y)
        Difference deltaIn;
        Difference deltaOut;
        bool found;
    };

    /**
     * Configuration des points distingués
     */
    struct Config {
        uint32_t distinguishedBitCount = 16;  // k bits de poids faible à zéro
        uint64_t maxWalkSteps = 10000;        // Limite d'itérations par marche
        uint64_t maxMarches = 1000000;        // Nombre total de marches
        uint32_t numThreads = std::thread::hardware_concurrency();
        Difference targetDeltaIn = 0x0001;    // Différence d'entrée à analyser
    };

    DistinguishedPoints(ToyCipher& cipher, const Config& config);
    ~DistinguishedPoints() = default;

    /**
     * Recherche de collisions avec parallélisation
     */
    std::vector<CollisionResult> findCollisions();

    /**
     * Afficher les statistiques
     */
    void printStatistics() const;

private:
    ToyCipher& cipher;
    Config config;

    // Données thread-safe pour les points distingués
    std::unordered_map<Block, DistinguishedPoint> distinguishedTable;
    mutable std::mutex tableWriteMutex;

    // Statistiques
    struct Stats {
        uint64_t totalWalks = 0;
        uint64_t collisionsFound = 0;
        uint64_t distinguishedPointsFound = 0;
        mutable std::mutex statsMutex;
    } stats;

    /**
     * Vérifie si une valeur est un "point distingué"
     * Condition : les k bits de poids faible sont à zéro
     */
    bool isDistinguished(Block value) const;

    /**
     * Fonction de marche : itère G(x) jusqu'à trouver un point distingué
     */
    DistinguishedPoint performWalk(Block startingPoint);

    /**
     * Fonction de génération G(x) = encrypt(x) XOR (x << rotation)
     */
    Block functionG(Block x) const;

    /**
     * Remonter à la collision exacte par réévaluation
     */
    CollisionResult traceback(
        Block x1, Block y1,
        const DistinguishedPoint& x1Point,
        const DistinguishedPoint& y1Point
    );

    /**
     * Worker thread : effectue les marches et recherche
     */
    void workerThread(uint32_t threadId, uint64_t marcheStart, uint64_t marcheEnd);
};

#endif

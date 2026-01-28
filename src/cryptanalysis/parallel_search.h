#ifndef PARALLEL_SEARCH_H
#define PARALLEL_SEARCH_H

#include "../utils/types.h"
#include "../cipher/toy_cipher.h"
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <cstdint>

/**
 * Recherche parallèle optimisée pour la cryptanalyse différentielle
 * Évite les faux partages (false sharing) en utilisant du padding
 */
class ParallelSearch {
public:
    struct Config {
        uint32_t numThreads = std::thread::hardware_concurrency();
        uint64_t samplesPerThread = 100000;
        Difference targetDeltaIn = 0x0001;
    };

    struct AlignedCounter {
        // Padding pour éviter les faux partages (false sharing)
        // Chaque counter occupe sa propre ligne de cache (64 octets typical)
        alignas(64) std::atomic<uint64_t> value{0};
    };

    ParallelSearch(ToyCipher& cipher, const Config& config);
    ~ParallelSearch() = default;

    /**
     * Recherche parallèle d'une différence d'entrée donnée
     */
    DifferentialCount searchDifferentialsParallel(Difference deltaIn);

    /**
     * Analyser plusieurs différences en parallèle
     */
    void analyzeMultipleDifferencesParallel(const std::vector<Difference>& deltaIns);

    /**
     * Afficher les statistiques
     */
    void printStatistics() const;

private:
    ToyCipher& cipher;
    Config config;

    // Résultats thread-safe avec alignement pour éviter le false sharing
    std::vector<std::unique_ptr<AlignedCounter>> alignedCounts;
    
    // Table globale des différentielles (protégée par mutex)
    DifferentialCount globalDifferentials;
    mutable std::mutex globalMutex;

    // Statistiques de performance
    struct PerformanceStats {
        std::atomic<uint64_t> totalSamples{0};
        std::atomic<uint64_t> totalDifferentials{0};
        std::atomic<double> timeElapsed{0.0};
        mutable std::mutex statsMutex;
    } perfStats;

    /**
     * Worker thread : effectue l'analyse différentielle sur une portion
     */
    void workerThread(
        uint32_t threadId,
        Difference deltaIn,
        uint64_t samplesStart,
        uint64_t samplesEnd
    );

    /**
     * Fonction F optimisée pour la parallélisation
     */
    Block functionF(Block right, uint32_t roundKey) const;
};

#endif

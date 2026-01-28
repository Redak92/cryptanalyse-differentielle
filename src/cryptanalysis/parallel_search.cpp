#include "parallel_search.h"
#include "../utils/utils.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <chrono>

ParallelSearch::ParallelSearch(ToyCipher& cipher, const Config& config)
    : cipher(cipher), config(config) {
    
    // Initialiser les compteurs alignés (évite le false sharing)
    alignedCounts.reserve(config.numThreads);
    for (uint32_t i = 0; i < config.numThreads; i++) {
        alignedCounts.push_back(std::make_unique<AlignedCounter>());
    }
}

Block ParallelSearch::functionF(Block right, uint32_t roundKey) const {
    // Fonction F du toy cipher (S-Box + rotation)
    Block temp = right ^ roundKey;
    
    // S-Box approximée (8 bits de substitution)
    static const uint8_t SBOX[256] = {
        0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
        0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
        0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
        0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
        0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
        0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
        0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
        0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
        0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
        0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
        0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
        0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
        0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xd7, 0x4b, 0x55, 0xcf, 0x34, 0xc5, 0x84,
        0xcb, 0xda, 0x5f, 0xce, 0x4f, 0xea, 0x72, 0x96, 0xb4, 0xd0, 0xfe, 0xf9, 0xb2, 0x45, 0x27, 0xb7,
        0xe0, 0xe6, 0xff, 0xb3, 0x33, 0x7d, 0xbd, 0x2b, 0x34, 0x34, 0xb9, 0xd0, 0x6d, 0xef, 0xac, 0x62,
        0x92, 0x95, 0xe4, 0x79, 0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea
    };

    Block result = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t byte = (temp >> (i * 8)) & 0xFF;
        result |= ((Block)SBOX[byte]) << (i * 8);
    }

    // Rotation circulaire
    result = (result << 7) | (result >> 25);
    return result;
}

void ParallelSearch::workerThread(
    uint32_t threadId,
    Difference deltaIn,
    uint64_t samplesStart,
    uint64_t samplesEnd) {
    
    // Buffer local pour accumuler les différentielles avant de les verrouiller
    std::unordered_map<Block, uint64_t> localBuffer;

    for (uint64_t i = samplesStart; i < samplesEnd; i++) {
        // Générer un plaintext pseudo-aléatoire
        Block x = static_cast<Block>(i ^ (0x12345678 ^ (i * 0x9E3779B9)));

        Block x1 = x;
        Block x2 = x ^ deltaIn;

        Block y1 = cipher.encrypt(x1);
        Block y2 = cipher.encrypt(x2);

        Difference deltaOut = y1 ^ y2;

        // Accumuler dans le buffer local (sans verrouillage)
        localBuffer[deltaOut]++;

        // Mise à jour locale du compteur aligné (accès concurrent léger)
        alignedCounts[threadId]->value++;
    }

    // Fusionner le buffer local avec la table globale (verrouillage une fois)
    {
        std::lock_guard<std::mutex> lock(globalMutex);
        for (const auto& entry : localBuffer) {
            DifferentialPair diff = {deltaIn, entry.first};
            globalDifferentials[diff] += entry.second;
        }
    }

    // Mettre à jour les stats globales
    perfStats.totalSamples += (samplesEnd - samplesStart);
    perfStats.totalDifferentials++;
}

DifferentialCount ParallelSearch::searchDifferentialsParallel(Difference deltaIn) {
    if (deltaIn == 0) {
        return DifferentialCount();
    }

    std::cout << "\n=== Recherche Parallèle Optimisée ===\n";
    std::cout << "Différence d'entrée : " << Utils::toHex(deltaIn) << "\n";
    std::cout << "Nombre de threads : " << config.numThreads << "\n";
    std::cout << "Samples par thread : " << config.samplesPerThread << "\n";

    auto startTime = std::chrono::high_resolution_clock::now();

    // Diviser le travail entre les threads
    uint64_t totalSamples = config.samplesPerThread * config.numThreads;
    std::vector<std::thread> workers;

    // Créer et lancer les worker threads
    for (uint32_t i = 0; i < config.numThreads; i++) {
        uint64_t samplesStart = i * config.samplesPerThread;
        uint64_t samplesEnd = samplesStart + config.samplesPerThread;

        workers.emplace_back(
            &ParallelSearch::workerThread,
            this,
            i,
            deltaIn,
            samplesStart,
            samplesEnd
        );
    }

    // Attendre que tous les threads se terminent
    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(endTime - startTime).count();
    perfStats.timeElapsed = elapsed;

    std::cout << "Temps d'exécution : " << std::fixed << std::setprecision(3) << elapsed << "s\n";
    std::cout << "Throughput : " << (totalSamples / elapsed / 1e6) << "M samples/sec\n";

    return globalDifferentials;
}

void ParallelSearch::analyzeMultipleDifferencesParallel(const std::vector<Difference>& deltaIns) {
    std::cout << "\nAnalyse parallèle de " << deltaIns.size() << " différences...\n";

    for (const auto& deltaIn : deltaIns) {
        searchDifferentialsParallel(deltaIn);
    }
}

void ParallelSearch::printStatistics() const {
    std::cout << "\n=== Statistiques Parallélisation ===\n";
    std::cout << "Total samples traités : " << perfStats.totalSamples << "\n";
    std::cout << "Temps total : " << std::fixed << std::setprecision(3) << perfStats.timeElapsed << "s\n";
    
    if (perfStats.timeElapsed > 0) {
        double throughput = perfStats.totalSamples.load() / perfStats.timeElapsed;
        std::cout << "Throughput : " << std::fixed << std::setprecision(2) << (throughput / 1e6) << " M samples/s\n";
    }

    std::cout << "Différentielles trouvées : " << globalDifferentials.size() << "\n";
    
    // Afficher les meilleures différentielles
    std::vector<std::pair<DifferentialPair, uint64_t>> sorted(
        globalDifferentials.begin(),
        globalDifferentials.end()
    );
    std::sort(sorted.begin(), sorted.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    std::cout << "\nTop 5 différentielles :\n";
    for (size_t i = 0; i < std::min(size_t(5), sorted.size()); i++) {
        double prob = static_cast<double>(sorted[i].second) / perfStats.totalSamples;
        std::cout << "  " << i + 1 << ". deltaOut = " << Utils::toHex(sorted[i].first.second)
                  << " | Prob = " << std::fixed << std::setprecision(6) << prob << "\n";
    }
}

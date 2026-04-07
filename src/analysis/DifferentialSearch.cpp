#include "DifferentialSearch.h"
#include <random>
#include <unordered_map>
#include <cmath>
#include <omp.h>
#include <unordered_set>
#include <algorithm>
#include <iostream>

// Constructeur
DifferentialSearch::DifferentialSearch(const ICipher &targetCipher) : cipher(targetCipher) {
}

[[nodiscard]] std::vector<DifferentialCandidate> DifferentialSearch::runBruteForceSearch(
    const double probabilityThreshold) const {
    const int n = cipher.getBlockSize();
    const uint64_t limit = 1ULL << n;
    const auto minCount = static_cast<uint64_t>(probabilityThreshold * static_cast<double>(limit));

    std::vector<Block> ftable(limit);
    for (uint64_t x = 0; x < limit; ++x)
        ftable[x] = cipher.encrypt(x);

    std::vector<DifferentialCandidate> results;

//#pragma omp parallel
    {
        std::vector<uint64_t> temp_counts(limit, 0);
        std::vector<DifferentialCandidate> localResults;

//#pragma omp for schedule(static)
        for (int64_t alpha = 1; alpha < static_cast<int64_t>(limit); ++alpha) {
            std::fill(temp_counts.begin(), temp_counts.end(), 0);

            for (uint64_t x = 0; x < limit; ++x)
                temp_counts[ftable[x] ^ ftable[x ^ static_cast<uint64_t>(alpha)]]++;

            for (uint64_t beta = 0; beta < limit; ++beta) {
                if (temp_counts[beta] > minCount) {
                    localResults.push_back({
                        static_cast<uint64_t>(alpha),
                        beta,
                        static_cast<double>(temp_counts[beta]) / static_cast<double>(limit)
                    });
                }
            }
        }

//#pragma omp critical
        {
            results.insert(results.end(), localResults.begin(), localResults.end());
        }
    }

    return results;
}

[[nodiscard]] std::vector<DifferentialCandidate> DifferentialSearch::runStandardSearch(
    const double probabilityThreshold) const {
    std::vector<DifferentialCandidate> globalResults;

    // 1. Taille de l'espace (2^n)
    const int n = cipher.getBlockSize();
    const uint64_t limit = 1ULL << n;
    const auto pairsPerDifference = static_cast<uint64_t>(4.0 / probabilityThreshold);

//#pragma omp parallel default(none) shared(globalResults, limit, pairsPerDifference)
    {
        // Générateur aléatoire
        std::random_device rd;
        std::mt19937_64 gen(rd() ^ omp_get_thread_num());
        std::uniform_int_distribution<uint64_t> dis(0, limit - 1);

        std::vector<DifferentialCandidate> localResults;

        // 2. On boucle sur toutes les différences possibles (alpha)
//#pragma omp for schedule(static)
        for (int64_t i = 1; i < static_cast<int64_t>(limit); ++i) {
            // Table pour compter les sorties beta pour cet alpha
            std::unordered_map<Difference, int> betaCounts;

            for (uint64_t k = 0; k < pairsPerDifference; ++k) {
                // Générer x et y
                const auto x = dis(gen);
                const Block y = x ^ static_cast<uint64_t>(i);

                // Chiffrer
                const Block c1 = cipher.encrypt(x);
                const Block c2 = cipher.encrypt(y);

                // Calculer la différence de sortie et compter
                betaCounts[c1 ^ c2]++;
            }

            double expectedHits = pairsPerDifference * probabilityThreshold;
            int minThreshold = std::max(3, static_cast<int>(expectedHits / 2));

            // 3. PHASE DE FILTRAGE
            for (const auto &[beta, count]: betaCounts) {
                // CRITÈRE DE FILTRAGE : Count > 1
                // Comme démontré dans le rapport, cela élimine le bruit (loi de Poisson).
                if (count > minThreshold) {
                    DifferentialCandidate candidate{};
                    candidate.alpha = static_cast<uint64_t>(i);
                    candidate.beta = beta;
                    candidate.probability = static_cast<double>(count) / static_cast<double>(pairsPerDifference);

                    localResults.push_back(candidate);
                }
            }
        }

//#pragma omp critical
        {
            globalResults.insert(globalResults.end(), localResults.begin(), localResults.end());
        }
    }

    return globalResults;
}

[[nodiscard]] std::vector<DifferentialCandidate> DifferentialSearch::runFundamentalAlgorithm(
    const double probabilityThreshold) const {
    const int n = cipher.getBlockSize();
    const uint64_t mask = (n < 64) ? (1ULL << n) - 1 : 0xFFFFFFFFFFFFFFFF;
    const auto M = static_cast<uint64_t>(std::sqrt(n) * std::pow(2.0, n / 2.0) / probabilityThreshold);
    const auto N_verify = static_cast<uint64_t>(n / probabilityThreshold);

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, mask);

    Block gamma;
    do { gamma = dis(gen); } while (gamma == 0);

    // hashmap : g_γ(x) : [(x, f(x)), ...]
    std::unordered_map<Block, std::vector<std::pair<Block, Block> > > hashmap;
    hashmap.reserve(M);

    std::unordered_map<uint64_t, uint32_t> counters;
    counters.reserve(M);

    /* ----------------------- Detection phase ----------------------- */

    for (uint64_t i = 0; i < M; ++i) {
        const Block x = dis(gen);
        const Block fx = cipher.encrypt(x);
        const Block gx = fx ^ cipher.encrypt(x ^ gamma);

        auto &bucket = hashmap[gx];
        for (const auto &[prev_x, prev_fx]: bucket) {
            const Block alpha = x ^ prev_x;
            if (alpha == 0) continue;
            const Block beta = fx ^ prev_fx;

            const uint64_t key = (static_cast<uint64_t>(alpha) << 32) | (static_cast<uint64_t>(beta) & 0xFFFFFFFFULL);
            counters[key]++;
        }
        bucket.emplace_back(x, fx);
    }

    /* ----------------------- Verification phase ----------------------- */

    std::vector<DifferentialCandidate> results;

    for (auto const &[key, count]: counters) {
        if (count < static_cast<uint32_t>(n / 4)) continue;

        const Block alpha = key >> 32;
        const Block beta = key & 0xFFFFFFFF;

        uint64_t hits = 0;
        for (uint64_t i = 0; i < N_verify; ++i) {
            if (const Block x = dis(gen); (cipher.encrypt(x) ^ cipher.encrypt(x ^ alpha)) == beta)
                ++hits;
        }

        if (hits > static_cast<uint64_t>(n / 2)) {
            results.push_back({
                alpha,
                beta,
                static_cast<double>(hits) / static_cast<double>(N_verify)
            });
        }
    }

    return results;
}

[[nodiscard]] std::vector<DifferentialCandidate> DifferentialSearch::runWorstCaseAlgorithm(
    const double probabilityThreshold) const {
    const int n = cipher.getBlockSize();
    const uint64_t mask = (n < 64) ? (1ULL << n) - 1 : 0xFFFFFFFFFFFFFFFF;

    const auto S = static_cast<uint64_t>(200.0 * n / probabilityThreshold);
    const auto M = static_cast<uint64_t>(4.0 * std::pow(2.0, n / 2.0) * std::pow(probabilityThreshold, -0.5));
    const auto threshold = static_cast<uint64_t>(0.28 * static_cast<double>(S) * probabilityThreshold);

    std::unordered_map<uint64_t, uint64_t> L;

//#pragma omp parallel
    {
        std::random_device rd;
        std::mt19937_64 gen(rd() ^ static_cast<uint64_t>(omp_get_thread_num()));
        std::uniform_int_distribution<uint64_t> gamDis(1, mask);
        std::uniform_int_distribution<uint64_t> xDis(0, mask);

//#pragma omp for schedule(dynamic)
        for (int64_t i = 0; i < static_cast<int64_t>(S); ++i) {
            const Block gamma = gamDis(gen);

            std::unordered_map<Block, std::vector<std::pair<Block, Block> > > hashmap;
            std::unordered_set<uint64_t> Ltmp;

            for (uint64_t j = 0; j < M; ++j) {
                const Block x = xDis(gen);
                const Block fx = cipher.encrypt(x);
                const Block gx = fx ^ cipher.encrypt(x ^ gamma);

                auto &bucket = hashmap[gx];
                for (const auto &[prev_x, prev_fx]: bucket) {
                    const Block alpha = x ^ prev_x;
                    if (alpha == 0) continue;
                    const Block beta = fx ^ prev_fx;
                    const uint64_t key = (static_cast<uint64_t>(alpha) << 32)
                                         | (static_cast<uint64_t>(beta) & 0xFFFFFFFFULL);
                    Ltmp.insert(key);
                }
                bucket.emplace_back(x, fx);
            }

//#pragma omp critical
            {
                for (const uint64_t key: Ltmp)
                    L[key]++;
            }
        }
    }

    std::vector<DifferentialCandidate> results;

    for (const auto &[key, cnt]: L) {
        if (cnt < threshold) continue;

        const Block alpha = key >> 32;
        const Block beta = key & 0xFFFFFFFFULL;

        results.push_back({
            alpha,
            beta,
            static_cast<double>(cnt) / static_cast<double>(S)
        });
    }

    return results;
}


[[nodiscard]] std::vector<DifferentialCandidate> DifferentialSearch::runMemoryEfficientAlgorithm(
    const double probabilityThreshold,
    uint64_t maxBatchSize) const {

    const int n = cipher.getBlockSize();
    const uint64_t mask = (n < 64) ? (1ULL << n) - 1 : 0xFFFFFFFFFFFFFFFF;

    // Calcul du M idéal de l'algorithme fondamental
    const double M_ideal = std::sqrt(n) * std::pow(2.0, n / 2.0) / probabilityThreshold;
    const auto N_verify = static_cast<uint64_t>(n / probabilityThreshold);
    
    uint64_t M_batch = static_cast<uint64_t>(M_ideal);
    uint64_t nb_batches = 1;

    if (M_ideal > maxBatchSize) {
        M_batch = maxBatchSize;
        // Si on divise la taille du tableau par K, le nombre de paires générées
        // chute de K^2. Il faut donc K^2 batches pour retrouver 100% de nos probabilités.
        double K = M_ideal / static_cast<double>(maxBatchSize);
        nb_batches = static_cast<uint64_t>(std::ceil(K * K));
    }

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, mask);

    // --- LE FILTRE DE FRÉQUENCE (1 Go de RAM fixe) ---
    const uint64_t FILTER_SIZE = 1024 * 1024 * 1024; // 1 Milliard de cases
    std::vector<uint8_t> frequency_filter(FILTER_SIZE, 0);
    
    // On ne stocke ici que les "vrais" candidats qui ont passé le filtre
    std::unordered_set<uint64_t> candidates_to_verify;
    
    struct Record { Block gx, x, fx; };

    for (uint64_t b = 0; b < nb_batches; ++b) {

        Block gamma;
        do { gamma = dis(gen); } while (gamma == 0);

        std::vector<Record> batch(M_batch);
        
        //#pragma omp parallel
        {
            std::mt19937_64 local_gen(rd() ^ omp_get_thread_num());
            std::uniform_int_distribution<uint64_t> local_dis(0, mask);

            //#pragma omp for schedule(static)
            for (int64_t i = 0; i < static_cast<int64_t>(M_batch); ++i) {
                const Block x = local_dis(local_gen);
                const Block fx = cipher.encrypt(x);
                const Block gx = fx ^ cipher.encrypt(x ^ gamma);
                batch[i] = {gx, x, fx};
            }
        }

        std::sort(batch.begin(), batch.end(), [](const Record& a, const Record& b) { return a.gx < b.gx; });

        for (size_t i = 0; i < M_batch; ) {
            size_t j = i + 1;
            while (j < M_batch && batch[j].gx == batch[i].gx) j++;
            
            size_t count = j - i;
            if (count > 1) {
                for (size_t a = i; a < j; ++a) {
                    for (size_t b = a + 1; b < j; ++b) {
                        const Block alpha = batch[a].x ^ batch[b].x;
                        if (alpha == 0) continue;
                        
                        const Block beta = batch[a].fx ^ batch[b].fx;
                        const uint64_t key = (static_cast<uint64_t>(alpha) << 32) | (static_cast<uint64_t>(beta) & 0xFFFFFFFFULL);

                        // On mélange les bits pour éviter les collisions de hachage
                        size_t hash_idx = std::hash<uint64_t>{}(key) % FILTER_SIZE;
                            
                        // On incrémente le filtre (sans dépasser 255)
                        if (frequency_filter[hash_idx] < 255) {
                                frequency_filter[hash_idx]++;
                        }
                            
                        // Si cette case a atteint n/4, on garde la vraie clé
                        if (frequency_filter[hash_idx] >= static_cast<uint8_t>(n / 4)) {
                            candidates_to_verify.insert(key);
                        }
                    }
                }
            }

            i = j;
        }
    }

    std::cout << std::endl;

    std::vector<DifferentialCandidate> results;

    for (uint64_t key : candidates_to_verify) {
        const Block alpha = key >> 32;
        const Block beta = key & 0xFFFFFFFF;

        uint64_t hits = 0;
        for (uint64_t i = 0; i < N_verify; ++i) {
            if (const Block x = dis(gen); (cipher.encrypt(x) ^ cipher.encrypt(x ^ alpha)) == beta)
                ++hits;
        }

        if (hits > static_cast<uint64_t>(n / 2)) {
            results.push_back({
                alpha,
                beta,
                static_cast<double>(hits) / static_cast<double>(N_verify)
            });
        }
    }

    return results;
}

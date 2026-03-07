#include "DifferentialSearch.h"
#include <random>
#include <unordered_map>
#include <cmath>
#include <omp.h>
#include <unordered_set>

// Constructeur
DifferentialSearch::DifferentialSearch(const ICipher &targetCipher) : cipher(targetCipher) {
}

[[nodiscard]] std::vector<DifferentialCandidate> DifferentialSearch::runStandardSearch(
    const double probabilityThreshold) const {
    std::vector<DifferentialCandidate> globalResults;

    // 1. Taille de l'espace (2^n)
    const int n = cipher.getBlockSize();
    const uint64_t limit = 1ULL << n;
    const auto pairsPerDifference = static_cast<uint64_t>(4.0 / probabilityThreshold);

#pragma omp parallel num_threads(omp_get_max_threads())
    {
        // Générateur aléatoire
        std::random_device rd;
        std::mt19937_64 gen(rd() ^ omp_get_thread_num());
        std::uniform_int_distribution<uint64_t> dis(0, limit - 1);

        std::vector<DifferentialCandidate> localResults;

        // 2. On boucle sur toutes les différences possibles (alpha)
#pragma omp for schedule(dynamic)
        for (uint64_t i = 1; i < limit; ++i) {

            // Table pour compter les sorties beta pour cet alpha
            std::unordered_map<Difference, int> betaCounts;

            for (uint64_t k = 0; k < pairsPerDifference; ++k) {
                // Générer x et y
                const auto x = dis(gen);
                const Block y = x ^ i;

                // Chiffrer
                const Block c1 = cipher.encrypt(x);
                const Block c2 = cipher.encrypt(y);

                // Calculer la différence de sortie et compter
                betaCounts[c1 ^ c2]++;
            }

            // 3. PHASE DE FILTRAGE
            for (const auto &[beta, count]: betaCounts) {
                // CRITÈRE DE FILTRAGE : Count > 1
                // Comme démontré dans le rapport, cela élimine le bruit (loi de Poisson).
                if (count > 1) {
                    DifferentialCandidate candidate{};
                    candidate.alpha = i;
                    candidate.beta = beta;
                    candidate.probability = static_cast<double>(count) / static_cast<double>(pairsPerDifference);

                    localResults.push_back(candidate);
                }
            }
        }

#pragma omp critical
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

        const auto alpha = key >> 32;
        const auto beta = key & 0xFFFFFFFF;

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

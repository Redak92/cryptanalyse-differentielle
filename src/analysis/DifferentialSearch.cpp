#include "DifferentialSearch.h"
#include <random>
#include <unordered_map>
#include <cmath>
#include <omp.h>

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
            const auto alpha = i;

            // Table pour compter les sorties beta pour cet alpha
            std::unordered_map<Difference, int> betaCounts;

            for (uint64_t k = 0; k < pairsPerDifference; ++k) {
                // Générer x et y
                const auto x = dis(gen);
                const Block y = x ^ alpha;

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
                    candidate.alpha = alpha;
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
    const auto M = static_cast<uint64_t>(sqrt(n) * pow(2, n / 2.0) / probabilityThreshold);
    const uint64_t mask = (n < 64) ? (1ULL << n) - 1 : 0xFFFFFFFFFFFFFFFF;

    std::unordered_map<Block, std::vector<Block> > hashmap;

    struct Pair {
        Block a, b;
        bool operator==(const Pair &o) const { return a == o.a && b == o.b; }
    };
    struct PairHash {
        size_t operator()(const Pair &p) const { return p.a ^ (p.b << 1); }
    };
    std::unordered_map<Pair, uint32_t, PairHash> counters;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, mask);

    Block gamma = dis(gen);
    if (gamma == 0) gamma = 1;

    /* ----------------------- Detection phase ----------------------- */

    for (uint64_t i = 0; i < M; ++i) {
        const Block x = dis(gen);
        const Block hashkey = computeDerivative(x, gamma);
        auto &list = hashmap[hashkey];

        if (!list.empty()) {
            for (const Block prev_x: list) {
                const Block alpha = x ^ prev_x;
                if (alpha == 0) continue;

                const Block beta = cipher.encrypt(x) ^ cipher.encrypt(prev_x);

                counters[{alpha, beta}]++;
            }
        }
        list.push_back(x);
    }

    /* ----------------------- Verification phase ----------------------- */

    std::vector<DifferentialCandidate> results;
    const auto N_verify = static_cast<uint64_t>(n / probabilityThreshold);

    for (auto const &[pair, count]: counters) {
        if (count >= static_cast<uint32_t>(n / 4)) {
            uint64_t verify_hits = 0;

            for (uint64_t i = 0; i < N_verify; ++i) {
                if (const Block x = dis(gen); (cipher.encrypt(x) ^ cipher.encrypt(x ^ pair.a)) == pair.b) {
                    verify_hits++;
                }
            }

            if (verify_hits > static_cast<uint64_t>(n / 2)) {
                const double final_probability = static_cast<double>(verify_hits) / static_cast<double>(N_verify);

                results.push_back({
                    pair.a,
                    pair.b,
                    final_probability
                });
            }
        }
    }

    return results;
}

Block DifferentialSearch::computeDerivative(const Block x, const Difference gamma) const {
    return cipher.encrypt(x) ^ cipher.encrypt(x ^ gamma);
}

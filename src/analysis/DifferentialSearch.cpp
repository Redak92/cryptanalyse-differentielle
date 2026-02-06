//
// Created by alexandre on 30/01/2026.
//

#include "DifferentialSearch.h"
#include <random>
#include <unordered_map>

// Constructeur
DifferentialSearch::DifferentialSearch(const ICipher& targetCipher) : cipher(targetCipher) {}

[[nodiscard]] std::vector<DifferentialCandidate> DifferentialSearch::runStandardSearch(const uint64_t pairsPerDifference) const {
    std::vector<DifferentialCandidate> results;

    // 1. Taille de l'espace (2^n)
    const int n = cipher.getBlockSize();
    const uint64_t limit = 1ULL << n;

    // Générateur aléatoire
    std::random_device rd;
    std::mt19937_64 gen(rd());
    // Distribution uniforme pour générer des blocs aléatoires
    std::uniform_int_distribution<uint64_t> dis(0, limit - 1);

    // 2. On boucle sur toutes les différences possibles (alpha)
    for (uint64_t i = 1; i < limit; ++i) {
        const auto alpha = static_cast<Difference>(i);

        // Table pour compter les sorties beta pour cet alpha
        std::unordered_map<Difference, int> betaCounts;

        // 3. On teste k paires aléatoires
        for (uint64_t k = 0; k < pairsPerDifference; ++k) {
            // Générer x et y
            const auto x = static_cast<Block>(dis(gen));
            const Block y = x ^ alpha;

            // Chiffrer
            const Block c1 = cipher.encrypt(x);
            const Block c2 = cipher.encrypt(y);

            // Calculer la différence de sortie et compter
            Difference beta = c1 ^ c2;
            betaCounts[beta]++;
        }

        // --- PHASE DE FILTRAGE ---
        for (const auto& [beta, count] : betaCounts) {
            // CRITÈRE DE FILTRAGE : Count > 1
            // Comme démontré dans le rapport, cela élimine le bruit (loi de Poisson).
            if (count > 1) {
                DifferentialCandidate candidate{};
                candidate.alpha = alpha;
                candidate.beta = beta;
                candidate.probability = static_cast<double>(count) / static_cast<double>(pairsPerDifference);

                results.push_back(candidate);
            }
        }
    }

    return results;
}

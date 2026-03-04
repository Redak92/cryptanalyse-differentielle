#pragma once

#include "../interfaces/ICipher.h"
#include <vector>

// Structure pour stocker une découverte
struct DifferentialCandidate {
    Difference alpha; // Input diff
    Difference beta; // Output diff
    double probability;
};

class DifferentialSearch {
    const ICipher &cipher; // Référence vers n'importe quel cipher (Speck ou Toy)

public:
    // On injecte le cipher à la construction
    explicit DifferentialSearch(const ICipher &targetCipher);

    // --- ALGO 1 : NAÏF ABSOLU ---
    // Complexité : O(2^{2n}). Teste toutes les paires (x, y) possibles.
    // Inutilisable si n > 16.
    std::vector<DifferentialCandidate> runBruteForceSearch();

    // --- ALGO 2 : NAÏF OPTIMISÉ ---
    // Complexité : O(2^n * p^-1).
    // Pour chaque différence d'entrée alpha possible, on teste k paires.
    [[nodiscard]] std::vector<DifferentialCandidate> runStandardSearch(double probabilityThreshold) const;

    // --- ALGO 3 : FONDAMENTAL ---
    // Complexité : O(2^{n/2} * p^-1).
    // Utilise la méthode Surrogate et les collisions.
    [[nodiscard]] std::vector<DifferentialCandidate> runFundamentalAlgorithm(double probabilityThreshold) const;

    // La fonction g_gamma (Section 2.2)
    [[nodiscard]] Block computeDerivative(Block x, Difference gamma) const;
};

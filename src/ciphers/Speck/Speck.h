//
// Created by alexandre on 30/01/2026.
//

#ifndef CRYPTANALYSE_DIFFERENTIELLE_SPECK_H
#define CRYPTANALYSE_DIFFERENTIELLE_SPECK_H

#include "interfaces/ICipher.h"
#include <vector>


class Speck : public ICipher {
public:
    /**
    * Constructeur Speck 32/64
    * Standard NSA : https://eprint.iacr.org/2013/404.pdf
    *
    * @param master_key Clé de 64 bits
    * @param rounds Nombre de tours (Standard = 22, mais modifiable pour la cryptanalyse (voir page Wikipédia))
    */
    explicit Speck(uint64_t master_key, int rounds = 22);

    // --- Interface ICipher ---
    [[nodiscard]] Block encrypt(Block plaintext) const override;
    [[nodiscard]] Block decrypt(Block ciphertext) const override;

    // Speck 32/64 a toujours une taille de bloc de 32 bits
    [[nodiscard]] int getBlockSize() const override { return 32; }

private:
    uint64_t master_key;
    int num_rounds;
    std::vector<uint16_t> round_keys;

    // Génération des sous-clés
    void scheduleKeys();

    // Rotations circulaires
    static uint16_t ror(const uint16_t x, const int r) {
        return (x >> r) | (x << (16 - r));
    }

    static uint16_t rol(const uint16_t x, const int r) {
        return (x << r) | (x >> (16 - r));
    }
};

#endif //CRYPTANALYSE_DIFFERENTIELLE_SPECK_H

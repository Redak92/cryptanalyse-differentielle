//
// Created by alexandre on 30/01/2026.
//

#include "Speck.h"
#include <cstdint>

// Constantes : Speck 32/64 : Alpha = 7, Beta = 2
static constexpr int ALPHA = 7;
static constexpr int BETA = 2;

Speck::Speck(const Key master_key, const int rounds) : master_key(master_key), num_rounds(rounds) {
    scheduleKeys();
}

void Speck::scheduleKeys() {
    round_keys.resize(num_rounds);

    // Découpage de la clé 64 bits en 4 mots de 16 bits
    auto k = static_cast<uint16_t>(master_key & 0xFFFF);
    uint16_t l[3];
    l[0] = static_cast<uint16_t>((master_key >> 16) & 0xFFFF);
    l[1] = static_cast<uint16_t>((master_key >> 32) & 0xFFFF);
    l[2] = static_cast<uint16_t>((master_key >> 48) & 0xFFFF);

    round_keys[0] = k;

    // Génération des clés suivantes
    for (int i = 0; i < num_rounds - 1; ++i) {
        // Étape 1 : Mise à jour de l[i]
        // ROR(l, ALPHA)
        const uint16_t l_curr = l[i % 3];
        const uint16_t l_rotated = ror(l_curr, ALPHA);

        // Addition modulaire avec k
        const auto l_added = static_cast<uint16_t>(l_rotated + k);

        // XOR avec le compteur de tour
        l[i % 3] = l_added ^ static_cast<uint16_t>(i);

        // Étape 2 : Mise à jour de k
        // ROL(k, BETA)
        const uint16_t k_rotated = rol(k, BETA);

        // XOR avec le nouveau l
        k = k_rotated ^ l[i % 3];

        round_keys[i + 1] = k;
    }
}

Block Speck::encrypt(const Block plaintext) const {
    auto x = static_cast<uint16_t>((plaintext >> 16) & 0xFFFF);
    auto y = static_cast<uint16_t>(plaintext & 0xFFFF);

    for (int i = 0; i < num_rounds; ++i) {
        // 1. Rotation Droite de ALPHA
        x = ror(x, ALPHA);

        // 2. Addition
        x += y;

        // 3. XOR Clé
        x ^= round_keys[i];

        // 4. Rotation Gauche de BETA
        y = rol(y, BETA);

        // 5. XOR Mélange
        y ^= x;
    }

    return (static_cast<Block>(x) << 16) | static_cast<Block>(y);
}

Block Speck::decrypt(const Block ciphertext) const {
    auto x = static_cast<uint16_t>((ciphertext >> 16) & 0xFFFF);
    auto y = static_cast<uint16_t>(ciphertext & 0xFFFF);

    for (int i = num_rounds - 1; i >= 0; --i) {
        // Inverse 5 : XOR
        y ^= x;

        // Inverse 4 : Rotation Droite de BETA (Inverse de ROL BETA)
        y = ror(y, BETA);

        // Inverse 3 : XOR Clé
        x ^= round_keys[i];

        // Inverse 2 : Soustraction
        x -= y;

        // Inverse 1 : Rotation Gauche de ALPHA (Inverse de ROR ALPHA)
        x = rol(x, ALPHA);
    }

    return (static_cast<Block>(x) << 16) | static_cast<Block>(y);
}

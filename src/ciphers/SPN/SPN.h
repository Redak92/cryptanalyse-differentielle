#pragma once

#include "../../interfaces/ICipher.h"
#include <vector>
#include <cstdint>

class SPN : public ICipher {
public :
    /**
     * Constructeur de SPN
     * Blocs de 16 bits,  Sbox de 8 bits, Pbox de 16 bits
     *
     * @param master_key Clé initiale
     * @param rounds Nombre de tours
     */

    explicit SPN(Key master_key, int rounds = 4);

    // --- Interface ICipher ---
    [[nodiscard]] Block encrypt(Block plaintext) const override;

    [[nodiscard]] Block decrypt(Block ciphertext) const override;

    [[nodiscard]] int getBlockSize() const override { return 16; }

private :
    Key master_key;
    int num_rounds;
    std::vector<Block> round_keys;

    // Génération des sous-clés
    void scheduleKeys();

    // --- Fonctions internes ---
    [[nodiscard]] Block applySbox(Block data, const uint8_t *table) const;

    static Block permutate(Block message);

    static Block inv_permutate(Block message);

    static constexpr uint32_t SBOX_SIZE = 16;

    // SBOX 4 bits fixes
    constexpr static uint8_t SBOX_ARRAY[SBOX_SIZE] = {
        0x0E, 0x04, 0x0D, 0x01,
        0x02, 0x0F, 0x0B, 0x08,
        0x03, 0x0A, 0x06, 0x0C,
        0x05, 0x09, 0x00, 0x07
    };

    constexpr static uint8_t INVERSE_SBOX_ARRAY[SBOX_SIZE] = {
        0x0E, 0x03, 0x04, 0x08,
        0x01, 0x0C, 0x0A, 0x0F,
        0x07, 0x0D, 0x09, 0x06,
        0x0B, 0x02, 0x00, 0x05
    };

    // 16 bits Pbox
    static constexpr uint32_t PBOX_SIZE = 16;

    constexpr static uint8_t PBOX_ARRAY[PBOX_SIZE] = {5, 9, 0, 13, 7, 2, 11, 14, 1, 4, 12, 8, 3, 15, 6, 10};
    constexpr static uint8_t INVERSE_PBOX_ARRAY[PBOX_SIZE] = {2, 8, 5, 12, 9, 0, 14, 4, 11, 1, 15, 6, 10, 3, 7, 13};
};

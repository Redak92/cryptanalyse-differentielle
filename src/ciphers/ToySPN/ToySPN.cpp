//
// Created by alexandre on 30/01/2026.
//

#include "ToySPN.h"

ToySPN::ToySPN(const Key master_key, const int rounds) : master_key(master_key), num_rounds(rounds) {
    scheduleKeys();
}

void ToySPN::scheduleKeys() {
    round_keys.resize(num_rounds);
    Block buffer = master_key;

    // Fait passer la clé rounds_nb fois par la Pbox
    for (int i = 0; i < num_rounds; i++) {
        round_keys.at(i) = permutate(buffer);
        buffer = round_keys.at(i);
    }
}

uint8_t ToySPN::substitute(const uint8_t message) {
    return SBOX_ARRAY[message];
}

uint8_t ToySPN::inv_substitute(const uint8_t message) {
    return INVERSE_SBOX_ARRAY[message];
}

Block ToySPN::permutate(const Block message) {
    Block result = 0;
    for (int i = 15; i >= 0; i--) {
        // masque pour isoler le bit selectionné par l'index
        const Block val = 0x0001 & (message >> i);
        // on décale le bit à sa nouvelle position
        result = val << PBOX_ARRAY[i] | result;
    }
    return result;
}

Block ToySPN::inv_permutate(const Block message) {
    Block result = 0;
    for (int i = 15; i >= 0; i--) {
        // masque pour isoler le bit selectionné par l'index
        const Block val = 0x0001 & (message >> i);
        // on décale le bit à sa nouvelle position
        result = val << INVERSE_PBOX_ARRAY[i] | result;
    }
    return result;
}


Block ToySPN::encrypt(Block plaintext) const {
    Block result = plaintext;

    for (int i = 0; i < num_rounds - 1; i++) {
        // XOR avec la clé du round actuel
        result = result ^ round_keys.at(i);

        // Division du block en deux
        uint8_t left = result >> 8;
        uint8_t right = result & 0x00FF;

        // Substitution par la SBOX
        left = substitute(left);
        right = substitute(right);

        // Reconstruction du block
        result = left << 8;
        result = result | right;

        // Permutation par la PBOX
        result = permutate(result);
    }

    // Dernier XOR
    return result ^ round_keys.at(ToySPN::num_rounds - 1);
}

Block ToySPN::decrypt(Block ciphertext) const {
    Block result = ciphertext;

    for (int i = num_rounds - 1; i > 0; i--) {
        // XOR avec la clé du round actuel
        result = result ^ round_keys.at(i);

        // Permutation par la PBOX
        result = inv_permutate(result);

        // Division du block en deux
        uint8_t left = result >> 8;
        uint8_t right = result & 0x00FF;

        // Substitution par la SBOX
        left = inv_substitute(left);
        right = inv_substitute(right);

        // Reconstruction du block
        result = left << 8;
        result = result | right;
    }

    // Dernier XOR
    return result ^ round_keys.at(0);
}

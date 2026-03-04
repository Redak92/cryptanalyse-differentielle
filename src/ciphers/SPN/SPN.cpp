#include "SPN.h"

SPN::SPN(const Key master_key, const uint8_t *sbox, const uint8_t *inv_sbox, int chunk_size, const int rounds)
    : master_key(master_key),
      num_rounds(rounds),
      sbox_chunk_size(chunk_size),
      sbox_table(sbox),
      inv_sbox_table(inv_sbox) {
    scheduleKeys();
}

void SPN::scheduleKeys() {
    round_keys.resize(num_rounds + 1);
    Block buffer = master_key;

    // Fait passer la clé rounds_nb fois par la Pbox
    for (int i = 0; i < num_rounds + 1; i++) {
        round_keys.at(i) = permutate(buffer);
        buffer = round_keys.at(i);
    }
}

Block SPN::permutate(const Block message) {
    Block result = 0;
    for (int i = 15; i >= 0; i--) {
        // masque pour isoler le bit selectionné par l'index
        const Block val = 0x0001 & (message >> i);
        // on décale le bit à sa nouvelle position
        result = val << PBOX_ARRAY[i] | result;
    }
    return result;
}

Block SPN::inv_permutate(const Block message) {
    Block result = 0;
    for (int i = 15; i >= 0; i--) {
        // masque pour isoler le bit selectionné par l'index
        const Block val = 0x0001 & (message >> i);
        // on décale le bit à sa nouvelle position
        result = val << INVERSE_PBOX_ARRAY[i] | result;
    }
    return result;
}

Block SPN::applySbox(const Block data, const uint8_t *table) const {
    Block result = 0;
    const Block mask = (1ULL << sbox_chunk_size) - 1;
    const int num_chunks = 16 / sbox_chunk_size;

    for (int i = 0; i < num_chunks; ++i) {
        const int shift = i * sbox_chunk_size;
        const uint16_t extract = (data >> shift) & mask;
        result |= (static_cast<Block>(table[extract]) << shift);
    }

    return result;
}

Block SPN::encrypt(const Block plaintext) const {
    Block result = plaintext;

    for (int i = 0; i < num_rounds; i++) {
        result ^= round_keys.at(i);
        result = applySbox(result, sbox_table);
        result = permutate(result);
    }

    return result ^ round_keys.at(num_rounds);
}

Block SPN::decrypt(const Block ciphertext) const {
    Block result = ciphertext;

    result ^= round_keys.at(num_rounds);

    for (int i = num_rounds - 1; i >= 0; i--) {
        result = inv_permutate(result);
        result = applySbox(result, inv_sbox_table);
        result ^= round_keys.at(i);
    }

    // Dernier XOR
    return result;
}

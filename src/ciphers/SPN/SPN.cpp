//
// Created by alexandre on 30/01/2026.
//
#include "SPN.h"

SPN::SPN(const Key master_key, const int rounds) : master_key(master_key), num_rounds(rounds) {
    scheduleKeys();
}

void SPN::scheduleKeys() {
    round_keys.resize(num_rounds);
    Block buffer = master_key;

    // Fait passer la clé rounds_nb fois par la Pbox
    for (int i = 0; i < num_rounds; i++) {
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


Block SPN::SBOXLayer(Block plaintext, bool inverse, bool reduceSbox) const {
    Block result = 0 ;
    // Déchiffrement de la SBOX 
    if(inverse){
        if(reduceSbox){
            for(int i = 4 ; i > 0 ; i--){
                Block buffer = plaintext >> 4*(i-1) ;    
                uint16_t var = buffer & 0x000F ; 
                result = result |( INVERSE_SBOX4_ARRAY[var] << 4*(i-1))  ;            
            } 
        } else {
            for(int i = 2 ; i > 0 ; i--){
                Block buffer = plaintext >> 8*(i-1) ;    
                uint16_t var = buffer & 0x00FF ; 
                result = result |( INVERSE_SBOX8_ARRAY[var] << 8*(i-1))  ;            
            }
        }
    } // Chiffrement de la SBOX 
    else{
        if(reduceSbox){
            for(int i = 4 ; i > 0 ; i--){
                Block buffer = plaintext >> 4*(i-1) ;    
                uint16_t var = buffer & 0x000F ; 
                result = result |( SBOX4_ARRAY[var] << 4*(i-1))  ;            
            } 
        } else {
            for(int i = 2 ; i > 0 ; i--){
                Block buffer = plaintext >> 8*(i-1) ;    
                uint16_t var = buffer & 0x00FF ; 
                result = result | ( SBOX8_ARRAY[var] << 8*(i-1))  ;            
            }
        }
    }
    return result ;
}

Block SPN::encrypt(Block plaintext) const {
    Block result = plaintext;

    for (int i = 0; i < num_rounds - 1; i++) {
        // XOR avec la clé du round actuel
        result = result ^ round_keys.at(i);

        result = SBOXLayer(result,0,REDUCE_SBOX_SIZE) ;

        // Permutation par la PBOX
        result = permutate(result);
    }

    // Dernier XOR
    return result ^ round_keys.at(SPN::num_rounds - 1);
}

Block SPN::decrypt(Block ciphertext) const {
    Block result = ciphertext;

    for (int i = num_rounds - 1; i > 0; i--) {
        // XOR avec la clé du round actuel
        result = result ^ round_keys.at(i);

        // Permutation par la PBOX
        result = inv_permutate(result);

        result = SBOXLayer(result,1,REDUCE_SBOX_SIZE) ;
    }

    // Dernier XOR
    return result ^ round_keys.at(0);
}
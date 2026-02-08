//
// Created by alexandre on 30/01/2026.
//

#include <iostream>
#include <vector>
#include <bitset>

#include "ToySPN.h"

#include "../../utils/KeySchedule.cpp"
#include "../../utils/Sbox.cpp"
#include "../../utils/Pbox.cpp"

using namespace std ;

// SPN avec clé de 32bits et une taille de bloc de 16 bits
uint16_t ToySPN::single_bloc(uint16_t message, uint32_t key, int tours){

    uint16_t result = message ;
    // Génération des round keys 
    vector<uint16_t> keys = KeySchedule::TEA_KeyS(key,tours) ;

    for(int i = 0 ; i < tours-1; i++){
        // XOR avec la clé du round actuel 
        result = result ^ keys.at(i) ;
        
        // Substitution par la SBOX 
        uint8_t left = result >> 8 ;
        uint8_t right = result & 0x0011 ;

        left = Sbox::substitute(left) ;
        right = Sbox::substitute(right) ;

        // Permutation par la PBOX
        result = left << 8 ;
        result = result | right ;

        result = Pbox::shuffle(result) ;
    }

    // Dernier XOR  
    return result ^ keys.at(tours-1) ;
}



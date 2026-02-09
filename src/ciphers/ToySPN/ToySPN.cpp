//
// Created by alexandre on 30/01/2026.
//

#include <iostream>
#include <vector>
#include <bitset>

#include "ToySPN.h"

using namespace std ;

ToySPN::ToySPN(int init_bloc_size, uint32_t init_key,int init_rounds_number)
    : block_size(init_bloc_size) , key(init_key), rounds_number(init_rounds_number)
{}
    
int ToySPN::getBlockSize() const {
    return ToySPN::block_size ;
}

int ToySPN::getRoundsNumber() const {
    return ToySPN::rounds_number ;
}

// SPN avec clé de 32bits et une taille de bloc de 16 bits
uint16_t ToySPN::encrypt(uint16_t message) const {

    uint16_t result = message ;
    // Génération des round keys 
    int rounds_nb = ToySPN::getRoundsNumber() ;
    vector<uint16_t> keys = KeySchedule::TEA_KeyS(ToySPN::key,rounds_nb) ;

    for(int i = 0 ; i < rounds_nb-1; i++){
        // XOR avec la clé du round actuel 
        result = result ^ keys.at(i) ;
        
        // Substitution par la SBOX 
        uint8_t left = result >> 8 ;
        uint8_t right = result & 0x00FF ;
        left = Sbox::encrypt(left) ;
        right = Sbox::encrypt(right) ;
        
        // Permutation par la PBOX
        result = left << 8 ;
        result = result | right ;
        result = Pbox::encrypt(result) ;
    }

    // Dernier XOR  
    return result ^ keys.at(rounds_nb-1) ;
}

uint16_t ToySPN::decrypt(uint16_t message) const {

    uint16_t result = message ;
    // Génération des round keys 
    int rounds_nb = ToySPN::getRoundsNumber() ;
    vector<uint16_t> keys = KeySchedule::TEA_KeyS(ToySPN::key,rounds_nb) ;

    for(int i = rounds_nb-1 ; i > 0 ; i--){
        // XOR avec la clé du round actuel 
        result = result ^ keys.at(i) ;
        
        // Permutation par la PBOX
        result = Pbox::decrypt(result) ;
        
        
        // Substitution par la SBOX 
        uint8_t left = result >> 8 ;
        uint8_t right = result & 0x00FF ;
        
        left = Sbox::decrypt(left) ;
        right = Sbox::decrypt(right) ;

        result = left << 8 ;
        result = result | right ;     
    }

    // Dernier XOR  
    return result ^ keys.at(0) ;
}

/* à effacer
int main(){
    
    ToySPN cypher(16,0x45678120,2) ;
    
    // int nb_tours = 2 ;
    // uint32_t key = 0x45678120 ;
    uint16_t message = 0x4567 ;
    
    cout << message << endl  ;
    uint16_t val = cypher.encrypt(message) ;
    cout << val << endl ;
    uint16_t nval = cypher.decrypt(val) ;
    cout << nval << endl ;

    return 0 ;
}
*/




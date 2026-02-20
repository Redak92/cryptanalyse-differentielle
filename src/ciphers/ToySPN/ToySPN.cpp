//
// Created by alexandre on 30/01/2026.
//
#include "ToySPN.h"

using namespace std ;

ToySPN::ToySPN(Key init_key,int init_rounds_number) : 
    key(init_key), rounds_number(init_rounds_number), round_keys(key_schedule(ToySPN::key,rounds_number)) {}

Block ToySPN::encrypt(Block message) const {

    Block result = message ;

    for(int i = 0 ; i < ToySPN::rounds_number-1; i++){
        // XOR avec la clé du round actuel 
        result = result ^ ToySPN::round_keys.at(i) ;
        
        // Division du block en deux 
        uint8_t left = result >> 8 ;
        uint8_t right = result & 0x00FF ;
        
        // Substitution par la SBOX
        left = substitute(left) ;
        right = substitute(right) ;
        
        // Reconstruction du block
        result = left << 8 ;
        result = result | right ;
        
        // Permutation par la PBOX
        result = permutate(result) ;
    }

    // Dernier XOR  
    return result ^ ToySPN::round_keys.at(ToySPN::rounds_number-1) ;
}

Block ToySPN::decrypt(Block message) const {

    Block result = message ;

    for(int i = ToySPN::rounds_number-1 ; i > 0 ; i--){
        // XOR avec la clé du round actuel 
        result = result ^ ToySPN::round_keys.at(i) ;
        
        // Permutation par la PBOX
        result = inv_permutate(result) ;
        
        // Division du block en deux 
        uint8_t left = result >> 8 ;
        uint8_t right = result & 0x00FF ;
        
        // Substitution par la SBOX
        left = inv_substitute(left) ;
        right = inv_substitute(right) ;

        // Reconstruction du block
        result = left << 8 ;
        result = result | right ;     
    }

    // Dernier XOR  
    return result ^ ToySPN::round_keys.at(0) ;
}

int ToySPN::getBlockSize() const {
    return ToySPN::BLOCK_SIZE ; 
}

uint8_t ToySPN::substitute(uint8_t message){
    return SBOX_ARRAY[message] ;
}

uint8_t ToySPN::inv_substitute(uint8_t message){
    return INVERSE_SBOX_ARRAY[message] ;
}

Block ToySPN::permutate(Block message){
    Block result = 0 ;    
    for(int i = 15; i >= 0; i-- ){
        // masque pour isoler le bit selectionné par l'index 
        Block val = 0x0001 & (message >> i) ;
        // on décale le bit à sa nouvelle position 
        result = val << PBOX_ARRAY[i] | result ;
    }
    return result ;
}

Block ToySPN::inv_permutate(Block message){
    Block result = 0 ;    
    for(int i = 15; i >= 0; i-- ){
        // masque pour isoler le bit selectionné par l'index 
        Block val = 0x0001 & (message >> i) ;
        // on décale le bit à sa nouvelle position 
        result = val << INVERSE_PBOX_ARRAY[i] | result ;
    }
    return result ;
}

vector<Block> ToySPN::key_schedule(Key key,int rounds_nb){
    vector<Block> keys(rounds_nb) ;
    Block buffer = key ;
    
    // Fait passer la clé rounds_nb fois par la Pbox
    for(int i = 0 ; i < rounds_nb; i++){
        keys.at(i) = permutate(buffer) ;       
        buffer = keys.at(i) ;
    }

    return keys ;
}
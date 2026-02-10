//
// Created by alexandre on 30/01/2026.
//
#include "../../utils/Types.h"
#include "../../interfaces/ICipher.h"

#ifndef CRYPTANALYSE_DIFFERENTIELLE_TOYSPN_H
#define CRYPTANALYSE_DIFFERENTIELLE_TOYSPN_H

// La taille d'un bloc est actuellement limité à 16 bits, soit la taille de la Pbox 

// Ajouter la possibilité de chiffrer des messages plus longs qu'un seul bloc (mode ECB)
class ToySPN : public ICipher{
    private : 
        int block_size = sizeof(Block) ;
        Key key = 0 ;    
        int rounds_number = 2 ;

    public : 
        ToySPN(int init_bloc_size) ;

        Block encrypt(Block message) const ;
        Block decrypt(Block message) const ;

        int getBlockSize() const ;

        void setRoundsNumber(int rounds_nb) ;
        void setKey(Key key) ;
};

class KeySchedule{
    public : 
    KeySchedule() ;

    // Key Schedule très rudimentaire, réutilisation de la Pbox à chaque tour
    static std::vector<Block> simplePermutation(Key key,int rounds_nb) ; 
} ;

class Sbox{
    // Sbox 8bit d'AES, initialisé avec la fontion initialize_aes_sbox issue de la page wikipedia de Rijndael S-box    
    private : 
    static const uint8_t SBOX_ARRAY[256] ;
    static const uint8_t INVERSE_SBOX_ARRAY[256] ;    
    
    public : 
    Sbox(); 
    static uint8_t encrypt(uint8_t message) ; 
    static uint8_t decrypt(uint8_t message) ; 


} ;

class Pbox{
    // Pbox de 16 bits de taille, bottleneck sur la taille des bloc
    private :
    static const uint8_t PBOX_ARRAY[16] ; 
    static const uint8_t INVERSE_PBOX_ARRAY[16] ;

    public:
    Pbox() ;
    static Block encrypt(Block message) ;
    static Block decrypt(Block message) ;

} ;

#endif //CRYPTANALYSE_DIFFERENTIELLE_TOYSPN_H
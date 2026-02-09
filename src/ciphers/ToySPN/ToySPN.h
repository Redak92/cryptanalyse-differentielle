//
// Created by alexandre on 30/01/2026.
//
#include "../../utils/KeySchedule.h"
#include "../../utils/TransformationArrays.h"
#include "../../interfaces/ICipher.h"

#ifndef CRYPTANALYSE_DIFFERENTIELLE_TOYSPN_H
#define CRYPTANALYSE_DIFFERENTIELLE_TOYSPN_H

// La taille d'un bloc est actuellement limité à 16 bits, soit la taille de la Pbox 

// Implémenter les types
// Ajouter la possibilité de chiffrer des messages plus longs qu'un bloc (mode ECB)
class ToySPN : public ICipher{
    private : 
        int block_size = 16 ;
        uint32_t key ;    
        int rounds_number = 2 ;

    public : 
        ToySPN(int init_bloc_size, uint32_t init_key, int init_rounds_number) ;

        uint16_t encrypt(uint16_t message) const ;
        uint16_t decrypt(uint16_t message) const ;

        int getBlockSize() const ;
        int getRoundsNumber() const ;
};

#endif //CRYPTANALYSE_DIFFERENTIELLE_TOYSPN_H
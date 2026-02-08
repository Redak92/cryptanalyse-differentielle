//
// Created by alexandre on 30/01/2026.
//

#ifndef CRYPTANALYSE_DIFFERENTIELLE_TOYSPN_H
#define CRYPTANALYSE_DIFFERENTIELLE_TOYSPN_H

// chiffrer des donnée de plus d'un bloc

class ToySPN {
    public : 
        ToySPN() ;
        // fonction temporaire 
        static uint16_t single_bloc(uint16_t message, uint32_t key, int tours) ;
};


#endif //CRYPTANALYSE_DIFFERENTIELLE_TOYSPN_H
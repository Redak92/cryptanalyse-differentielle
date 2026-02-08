#ifndef KEY_SCHEDULE
#define KEY_SCHEDULE

#include <vector>

// Inlcure la possibilité de décider combien de round keys vont être générer  
class KeySchedule{
    public : 
    KeySchedule() ;

    // Key Schedule très rudimentaire, peu efficace après deux tours à cause de la taille de la clé 
    static std::vector<uint16_t> TEA_KeyS(uint32_t key,int nb_tours) ; 
} ;

#endif
#include <iostream>
#include <vector>
#include "KeySchedule.h"

using namespace std ; 

vector<uint16_t> KeySchedule::TEA_KeyS(uint32_t key,int nb_tours){
    vector<uint16_t> keys(nb_tours) ;
    
    // alterne entre la première et deuxième moitié de la clé maitresse  
    for(int i = 0 ; i < nb_tours; i++){
        keys.at(i) = (i%2 == 0) ? (0x00001111 & key) : (key >> 16 ) ;       
    }

    return keys ;
} 

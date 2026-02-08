#include <iostream>
#include "Pbox.h"
using namespace std ;

const uint16_t Pbox::PBOX_ARRAY[16] = {5,9,0,13,7,2,11,14,1,4,12,8,3,15,6,10};    

uint16_t Pbox::shuffle(uint16_t message){
    uint16_t result = 0 ;    
    for(int i = 15; i >= 0; i-- ){
        
        uint16_t val = 0x0001 & (message >> i) ;
        
        result = val << Pbox::PBOX_ARRAY[i] | result ;
    }
    return result ;
}

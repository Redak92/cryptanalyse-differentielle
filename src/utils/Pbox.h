#ifndef PBOX
#define PBOX

class Pbox{
    private :
    static const uint16_t PBOX_ARRAY[16] ; 

    public:
    Pbox() ;
    static uint16_t shuffle(uint16_t message) ;
} ;

#endif 


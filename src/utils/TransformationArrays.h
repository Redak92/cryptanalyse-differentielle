#ifndef TRANSFORMATION_ARRAYS
#define TRANSFORMATION_ARRAYS

// On ajoutera la table inverse et la fonction inv_substitute plus tard 
class Sbox{
    private : 
    // Sbox 8bit d'AES, initialisé avec la fontion initialize_aes_sbox issue de la page wikipedia de Rijndael S-box    
    static const uint8_t SBOX_ARRAY[256] ;
    
    public : 
    Sbox(); 
    // Fonction de substitution 
    static uint8_t substitute(uint8_t message) ; 

} ;

class Pbox{
    private :
    static const uint16_t PBOX_ARRAY[16] ; 

    public:
    Pbox() ;
    static uint16_t shuffle(uint16_t message) ;
} ;

#endif
#ifndef TRANSFORMATION_ARRAYS
#define TRANSFORMATION_ARRAYS

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
    // Pbox de 16 bits de taille, il faudra l'agrandir pour travailler sur des blocs plus grand 
    private :
    static const uint16_t PBOX_ARRAY[16] ; 
    static const uint16_t INVERSE_PBOX_ARRAY[16] ;

    public:
    Pbox() ;
    static uint16_t encrypt(uint16_t message) ;
    static uint16_t decrypt(uint16_t message) ;

} ;

#endif
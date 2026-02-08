#ifndef SBOX
#define SBOX

// Possibilité de consolider Pbox et Sbox en une seule classe 

// On ajoutera la table inverse et la fonction inv_substitute plus tard 
class Sbox{
    private : 
    // Sbox 8bit d'AES, initialisé avec la fontion initialize_aes_sbox ci-dessus    
    static const uint8_t SBOX_ARRAY[256] ;
    
    public : 
    Sbox(); 
    // Fonction de substitution 
    static uint8_t substitute(uint8_t message) ; 

} ;

// Utilisé uniquement pour initialiser la sbox, issue de la page wikipedia de Rijndael S-box
// void initialize_aes_sbox(uint8_t sbox[256]) ;

#endif 
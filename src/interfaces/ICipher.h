//
// Created by alexandre on 30/01/2026.
//

#ifndef CRYPTANALYSE_DIFFERENTIELLE_ICIPHER_H
#define CRYPTANALYSE_DIFFERENTIELLE_ICIPHER_H

#include <cstdint>
#include "../utils/Types.h"

class ICipher {
public:
    virtual ~ICipher() = default;

    // Méthode que chaque cipher DOIT implémenter
    virtual Block encrypt(Block plaintext) const = 0;
    virtual Block decrypt(Block ciphertext) const = 0;

    // Pour savoir si on attaque du 16 bits, 32 bits, etc.
    virtual int getBlockSize() const = 0;
};


#endif //CRYPTANALYSE_DIFFERENTIELLE_ICIPHER_H
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
    [[nodiscard]] virtual Block encrypt(Block plaintext) const = 0;
    [[nodiscard]] virtual Block decrypt(Block ciphertext) const = 0;

    // Pour savoir si on attaque du 16 bits, 32 bits, etc.
    [[nodiscard]] virtual int getBlockSize() const = 0;
    
    // Alias pour compatibilité avec l'analyse différentielle
    [[nodiscard]] virtual uint32_t block_size_bits() const { return static_cast<uint32_t>(getBlockSize()); }
};


#endif //CRYPTANALYSE_DIFFERENTIELLE_ICIPHER_H
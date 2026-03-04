//
// Created by alexandre on 30/01/2026.
//

#pragma once

#include "../utils/Types.h"

class ICipher {
public:
    virtual ~ICipher() = default;

    // Méthode que chaque cipher DOIT implémenter
    [[nodiscard]] virtual Block encrypt(Block plaintext) const = 0;
    [[nodiscard]] virtual Block decrypt(Block ciphertext) const = 0;

    // Pour savoir si on attaque du 16 bits, 32 bits, etc.
    [[nodiscard]] virtual int getBlockSize() const = 0;
};

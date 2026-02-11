//
// Created by alexandre on 30/01/2026.
//

#ifndef CRYPTANALYSE_DIFFERENTIELLE_TYPES_H
#define CRYPTANALYSE_DIFFERENTIELLE_TYPES_H

#include <cstdint>

// Utilisation de uint64_t pour supporter des blocs jusqu'à 64 bits.
// Pour SPECK complet et autres ciphers larges.
using Block = uint64_t;
using Difference = uint64_t;
using Key = uint64_t;

#endif //CRYPTANALYSE_DIFFERENTIELLE_TYPES_H
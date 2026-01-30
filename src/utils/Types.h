//
// Created by alexandre on 30/01/2026.
//

#ifndef CRYPTANALYSE_DIFFERENTIELLE_TYPES_H
#define CRYPTANALYSE_DIFFERENTIELLE_TYPES_H

#include <cstdint>

// Pour l'instant, on fixe à 16 bits pour les jouets.
// Pour SPECK complet, il faudra peut-être passer à uint64_t.
using Block = uint16_t;
using Difference = uint16_t;
using Key = uint16_t;  // Ou plus grand selon le Key Schedule

#endif //CRYPTANALYSE_DIFFERENTIELLE_TYPES_H
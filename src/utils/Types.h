//
// Created by alexandre on 30/01/2026.
//

#pragma once

#include <cstdint>

// Utilisation de uint64_t pour supporter des blocs jusqu'à 64 bits.
// Pour SPECK complet et autres ciphers larges.
using Block = uint64_t;
using Difference = uint64_t;
using Key = uint64_t;

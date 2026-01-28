#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <vector>
#include <map>


using Block = uint32_t;                    
using Key = uint32_t;                      
using Difference = uint32_t;               
using DifferentialPair = std::pair<Difference, Difference>;  
using DifferentialCount = std::map<DifferentialPair, uint64_t>;  

#endif 

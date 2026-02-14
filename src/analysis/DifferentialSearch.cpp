//
// Created by alexandre on 30/01/2026.
//

#include "DifferentialSearch.h"
#include <random>
#include <unordered_map>
#include <cmath>
#include <iostream>
#include <unordered_set>

// Constructeur
DifferentialSearch::DifferentialSearch(const ICipher& targetCipher) : cipher(targetCipher) {}

[[nodiscard]] std::vector<DifferentialCandidate> DifferentialSearch::runStandardSearch(const uint64_t pairsPerDifference) const {
    std::vector<DifferentialCandidate> results;

    // 1. Taille de l'espace (2^n)
    const int n = cipher.getBlockSize();
    const uint64_t limit = 1ULL << n;

    // Générateur aléatoire
    std::random_device rd;
    std::mt19937_64 gen(rd());
    // Distribution uniforme pour générer des blocs aléatoires
    std::uniform_int_distribution<uint64_t> dis(0, limit - 1);

    // 2. On boucle sur toutes les différences possibles (alpha)
    for (uint64_t i = 1; i < limit; ++i) {
        const auto alpha = static_cast<Difference>(i);

        // Table pour compter les sorties beta pour cet alpha
        std::unordered_map<Difference, int> betaCounts;

        // 3. On teste k paires aléatoires
        for (uint64_t k = 0; k < pairsPerDifference; ++k) {
            // Générer x et y
            const auto x = static_cast<Block>(dis(gen));
            const Block y = x ^ alpha;

            // Chiffrer
            const Block c1 = cipher.encrypt(x);
            const Block c2 = cipher.encrypt(y);

            // Calculer la différence de sortie et compter
            Difference beta = c1 ^ c2;
            betaCounts[beta]++;
        }

        // --- PHASE DE FILTRAGE ---
        for (const auto& [beta, count] : betaCounts) {
            // CRITÈRE DE FILTRAGE : Count > 1
            // Comme démontré dans le rapport, cela élimine le bruit (loi de Poisson).
            if (count > 1) {
                DifferentialCandidate candidate{};
                candidate.alpha = alpha;
                candidate.beta = beta;
                candidate.probability = static_cast<double>(count) / static_cast<double>(pairsPerDifference);

                results.push_back(candidate);
            }
        }
    }

    return results;
}

std::vector<DifferentialCandidate> DifferentialSearch::runFundamentalAlgorithm(uint64_t numSamples){

    // probability threshold ---------------- (!)
    const double P_VALUE = 0.1 ;

    const uint64_t N_BITS = cipher.getBlockSize() ;
    
    // Variable sample size 
    const double M = sqrt(N_BITS) * pow(2,N_BITS/2) * (1/P_VALUE) ;

    std::unordered_map<uint32_t,std::vector<uint32_t>> hashmap ;
    // bitshift mask
    uint64_t mask = (1ULL << N_BITS) - 1;

    // detection_phase
    
    // counters 
    std::unordered_map<uint32_t,uint32_t> counters ;

    // results 
    std::vector<DifferentialCandidate> differentials ;

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0,pow(2,N_BITS));

    // Select a gamma 
    Block gamma = dis(gen) ;

    // generate unique samples 
    std::unordered_set<Block> initial_samples;
    initial_samples.reserve(M);

    while (initial_samples.size() < M) {
        initial_samples.insert(dis(gen));
    }

    for(Block x : initial_samples){    

        Block hashkey = computeDerivative(x,gamma) ;
        // Add x in the hash map based on the value of g(x) 
        hashmap[hashkey].push_back(x);
        // If there's more than one element we have a collision 
        if (hashmap[hashkey].size() > 1 ){
            for(int i = 0 ; i < hashmap[hashkey].size()-1 ; i++){
                // Determining alpha and beta and incrementing their value in counters 
                Block alpha = x ^ hashmap[hashkey][i] ;
                Block beta  = cipher.encrypt(x) ^ cipher.encrypt(hashmap[hashkey][i]) ;
                
                // Determining index based on alpha and beta 
                size_t index = static_cast<size_t>(((alpha & mask) << N_BITS) | (beta & mask));
                
                counters[index]++ ;
            }
        };
    }

    // verification_phase
    
    // generate a new set of unique samples 
    std::unordered_set<Block> additional_samples;
    additional_samples.reserve(N_BITS/P_VALUE);

    while (additional_samples.size() < N_BITS/P_VALUE) {
        additional_samples.insert(dis(gen));
    }
    
    for(auto const& [k,v] : counters){
        
        // If count surpasses a threshold ---------------- (!)
        if( v > 0){
            int count = 0 ;
            // Extract alpha and beta from the index 
            Block alpha  = (k >> N_BITS) ;
            Block beta = k & mask ;
            // Check the differences against the new dataset
            for(Block x : additional_samples)
                if(computeDerivative(x,alpha) == beta)
                    count++ ;
            
            // If the new count surpasses a threshold ---------------- (!)
            if(count > N_BITS/8){
                DifferentialCandidate differential{ .alpha = alpha, .beta = beta, .probability = count/(N_BITS/P_VALUE) } ;
                differentials.push_back(differential) ;
            }
        }
        
    }
    return differentials ;
}

Block DifferentialSearch::computeDerivative(Block x, Difference gamma){
    return cipher.encrypt(x) ^ cipher.encrypt(x ^ gamma) ;
}

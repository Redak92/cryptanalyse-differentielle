//
// Created by alexandre on 30/01/2026.
//

#include "DifferentialSearch.h"
#include <random>
#include <unordered_map>
#include <cmath>
#include <iostream>

// Constructeur
DifferentialSearch::DifferentialSearch(const ICipher& targetCipher) : cipher(targetCipher) {
    int N_BITS = cipher.getBlockSize() ;
    P_VALUE = pow(2,-(N_BITS/2)) ;
    SAMPLE_SIZE = sqrt(N_BITS) * pow(2,N_BITS/2) * (1/P_VALUE) ;
}

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

[[nodiscard]] std::vector<DifferentialCandidate> DifferentialSearch::runFundamentalAlgorithm(uint64_t numSamples) const{

    // size of the block 
    const uint64_t N_BITS = cipher.getBlockSize() ;
    // bitshift mask
    uint64_t mask = (1ULL << N_BITS) - 1;

    // (g(x),x) hashmap used to detect collisions between values  
    std::unordered_map<uint32_t,std::vector<uint32_t>> hashmap ;
    // ( concatenation( alpha , beta )  , count ) hashmap   
    std::unordered_map<uint32_t,uint32_t> counters ;
    // results 
    std::vector<DifferentialCandidate> differentials ;

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0,pow(2,N_BITS));

    // Selecting a difference gamma 
    Block gamma = dis(gen) ;

    /* ----------------------- Detection phase ----------------------- */  

    for( int i = 0 ; i < SAMPLE_SIZE; i++){
        // generate SAMPLE_SIZE unique samples 
        Block x = dis(gen) ;  

        Block hashkey = computeDerivative(x,gamma) ;
        // Add x in the hash map based on the value of g(x) 
        hashmap[hashkey].push_back(x);
        // If there's more than one element we have a collision 
        if (hashmap[hashkey].size() > 1 ){
            for(int j = 0 ; j < hashmap[hashkey].size()-1 ; j++){
                // Determining alpha and beta and incrementing their counter 
                Block alpha = x ^ hashmap[hashkey][j] ;
                Block beta  = cipher.encrypt(x) ^ cipher.encrypt(hashmap[hashkey][j]) ;
                
                // Determining index based on alpha and beta 
                size_t index = static_cast<size_t>(((alpha & mask) << N_BITS) | (beta & mask));
                
                counters[index]++ ;
            }
        };
    }

    /* ----------------------- Verification phase ----------------------- */  
    
    for(auto const& [k,v] : counters){
        
        // If count surpasses an n/4 threshold
        if( v >= N_BITS/4){
            int count = 0 ;
            // Extract alpha and beta from the index 
            Block alpha  = (k >> N_BITS) ;
            Block beta = k & mask ;
            
            // Check the differences against the new dataset
            for(int i = 0 ; i < N_BITS/P_VALUE ; i++){
                Block x = dis(gen) ;
                if(computeDerivative(x,alpha) == beta)
                    count++ ;
            }
                
            // If the new counter surpasses an n/2 threshold
            if(count > N_BITS/2){
                DifferentialCandidate differential{ .alpha = alpha, .beta = beta, .probability = count/(N_BITS/P_VALUE) } ;
                differentials.push_back(differential) ;
            }
        }
    }

    return differentials ;
}

Block DifferentialSearch::computeDerivative(Block x, Difference gamma) const{
    return cipher.encrypt(x) ^ cipher.encrypt(x ^ gamma) ;
}
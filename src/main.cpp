#include <iostream>
#include "cipher/toy_cipher.h"
#include "cryptanalysis/differential_search.h"
#include "utils/utils.h"
#include "utils/types.h"

int main() {
    std::cout << "=== Recherche de différentielles - Étape 3 ===\n\n";
    
    
    Key key = 0x12345678;
    int numRounds = 4;
    ToyCipher cipher(key, numRounds);
    
    std::cout << "Chiffrement jouet créé :\n";
    std::cout << "  Clé       : " << Utils::toHex(key) << "\n";
    std::cout << "  Rounds    : " << numRounds << "\n\n";
    
    
    Block plaintext = 0x12345678;
    Block ciphertext = cipher.encrypt(plaintext);
    Block decrypted = cipher.decrypt(ciphertext);
    
    std::cout << "Test de chiffrement/déchiffrement :\n";
    std::cout << "  Texte clair     : " << Utils::toHex(plaintext) << "\n";
    std::cout << "  Texte chiffré   : " << Utils::toHex(ciphertext) << "\n";
    std::cout << "  Déchiffré       : " << Utils::toHex(decrypted) << "\n";
    std::cout << "  Vérifié         : " << (plaintext == decrypted ? "OK" : "ERREUR") << "\n\n";
    
    
    uint64_t numSamples = 100000;  
    DifferentialSearch searcher(cipher, numSamples);
    
    std::cout << "Recherche de différentielles :\n";
    std::cout << "  Nombre d'échantillons : " << numSamples << "\n\n";
    
    
    std::vector<Difference> deltaIns = {
        0x00000001,  
        0x00000080,  
        0x00008000,  
        0x80000000,  
        0x000000FF,  
    };
    
    searcher.analyzeMultipleDifferences(deltaIns);
    
    
    std::cout << "\n=== Top 10 Différentielles ===\n";
    auto bestDiffs = searcher.findBestDifferentials(10, 0.0001);
    
    for (size_t i = 0; i < bestDiffs.size(); i++) {
        std::cout << std::fixed << std::setprecision(6);
        std::cout << (i + 1) << ". deltaIn  = " << Utils::toHex(bestDiffs[i].first.first) 
                  << ", deltaOut = " << Utils::toHex(bestDiffs[i].first.second)
                  << ", Prob = " << bestDiffs[i].second << "\n";
    }
    
    
    searcher.printStatistics();
    
    std::cout << "\n=== Fin de l'étape 3 ===\n";
    
    return 0;
}

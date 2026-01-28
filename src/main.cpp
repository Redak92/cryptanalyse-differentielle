#include <iostream>
#include <io.h>
#include <fcntl.h>
#include "cipher/toy_cipher.h"
#include "cryptanalysis/differential_search.h"
#include "utils/utils.h"
#include "utils/types.h"

int main() {
    // Disable UTF-8 mode to avoid encoding issues on Windows
    #ifdef _WIN32
        _setmode(_fileno(stdout), _O_TEXT);
    #endif
    
    std::cout << "=== Differential Analysis - Stage 3 ===\n\n";
    
    
    Key key = 0x12345678;
    int numRounds = 4;
    ToyCipher cipher(key, numRounds);
    
    std::cout << "Toy Cipher created:\n";
    std::cout << "  Key       : " << Utils::toHex(key) << "\n";
    std::cout << "  Rounds    : " << numRounds << "\n\n";
    
    
    Block plaintext = 0x12345678;
    Block ciphertext = cipher.encrypt(plaintext);
    Block decrypted = cipher.decrypt(ciphertext);
    
    std::cout << "Encryption/Decryption Test:\n";
    std::cout << "  Plaintext       : " << Utils::toHex(plaintext) << "\n";
    std::cout << "  Ciphertext      : " << Utils::toHex(ciphertext) << "\n";
    std::cout << "  Decrypted       : " << Utils::toHex(decrypted) << "\n";
    std::cout << "  Verified        : " << (plaintext == decrypted ? "OK" : "ERROR") << "\n\n";
    
    
    uint64_t numSamples = 100000;  
    DifferentialSearch searcher(cipher, numSamples);
    
    std::cout << "Differential Search:\n";
    std::cout << "  Sample count : " << numSamples << "\n\n";
    
    
    std::vector<Difference> deltaIns = {
        0x00000001,  
        0x00000080,  
        0x00008000,  
        0x80000000,  
        0x000000FF,  
    };
    
    searcher.analyzeMultipleDifferences(deltaIns);
    
    
    std::cout << "\n=== Top 10 Differentials ===\n";
    auto bestDiffs = searcher.findBestDifferentials(10, 0.0001);
    
    for (size_t i = 0; i < bestDiffs.size(); i++) {
        std::cout << std::fixed << std::setprecision(6);
        std::cout << (i + 1) << ". deltaIn  = " << Utils::toHex(bestDiffs[i].first.first) 
                  << ", deltaOut = " << Utils::toHex(bestDiffs[i].first.second)
                  << ", Prob = " << bestDiffs[i].second << "\n";
    }
    
    
    searcher.printStatistics();
    
    std::cout << "\n=== End of Stage 3 ===\n";
    
    return 0;
}

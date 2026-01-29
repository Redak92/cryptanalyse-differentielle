#include <iostream>
#include <io.h>
#include <fcntl.h>
#ifdef _WIN32
    #include <windows.h>
#endif
#include "cipher/toy_cipher.h"
#include "cryptanalysis/differential_search.h"
#include "utils/utils.h"
#include "utils/types.h"

int main() {
    #ifdef _WIN32
        SetConsoleCP(65001);
        SetConsoleOutputCP(65001);
    #endif

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

    DistinguishedPoints::Config dpConfig;
    dpConfig.numThreads = 4;
    dpConfig.maxMarches = numSamples;
    dpConfig.distinguishedBitCount = 16;
    dpConfig.maxWalkSteps = 5000;
    dpConfig.targetDeltaIn = 0x0001;
    
    auto collisions = searcher.findCollisionsWithDistinguishedPoints(dpConfig);

    std::cout << "\n=== Collisions trouvées ===\n";
    if (collisions.empty()) {
        std::cout << "Aucune collision trouvée\n";
    } else {
        std::cout << "Total: " << collisions.size() << " collisions\n\n";
        for (size_t i = 0; i < collisions.size() && i < 10; i++) {
            std::cout << (i + 1) << ". deltaIn  = " << Utils::toHex(collisions[i].deltaIn) 
                      << ", deltaOut = " << Utils::toHex(collisions[i].deltaOut) << "\n";
        }
        if (collisions.size() > 10) {
            std::cout << "... et " << (collisions.size() - 10) << " autres\n";
        }
    }

    searcher.printStatistics();

    std::cout << "\n=== End of Stage 3 ===\n";

    return 0;
}

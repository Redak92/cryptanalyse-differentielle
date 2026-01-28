#include "cipher/toy_cipher.h"
#include "cryptanalysis/differential_search.h"
#include "cryptanalysis/parallel_search.h"
#include "cryptanalysis/distinguished_points.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== Cryptanalyse Différentielle Optimisée ===\n\n";

    
    ToyCipher cipher(0xDEADBEEF, 4);

    
    std::cout << "Test 1 : Recherche différentielle basique\n";
    DifferentialSearch searcher(cipher, 100000);
    DifferentialCount results = searcher.searchDifferentials(0x0001);
    std::cout << "Différentielles trouvées : " << results.size() << "\n\n";

    
    std::cout << "Test 2 : Recherche parallélisée\n";
    DifferentialCount parallelResults = searcher.searchDifferentialsParallel(0x0001, 4);
    std::cout << "Différentielles trouvées (parallèle) : " << parallelResults.size() << "\n\n";

    
    std::cout << "Test 3 : Recherche par Points Distingués\n";
    DistinguishedPoints::Config dpConfig;
    dpConfig.numThreads = 4;
    dpConfig.maxMarches = 100000;
    dpConfig.distinguishedBitCount = 16;
    dpConfig.maxWalkSteps = 5000;

    auto collisions = searcher.findCollisionsWithDistinguishedPoints(dpConfig);
    std::cout << "Collisions trouvées : " << collisions.size() << "\n\n";

    
    searcher.printStatistics();

    std::cout << "\n=== Fin de l'analyse ===\n";
    return 0;
}

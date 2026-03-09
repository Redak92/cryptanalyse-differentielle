/**
 * @file benchmark_exhaustive_2.cpp
 * @brief Estimation du temps d'exécution de l'algorithme exhaustif
 * 
 * Utilise la bibliothèque Regression pour modéliser le temps sous la forme:
 * T(n) = a + b * 2^(2n)
 */

#include "../src/Regression/Regression.h"
#include <iostream>
#include <vector>
#include <string>

using namespace diffcrypto::regression;

int main(int argc, char* argv[])
{
    // Valeurs par défaut
    std::vector<int> test_n_values = {6, 8, 10, 12};
    int n_target = 24;
    
    // Parser l'argument CLI pour n_target
    if (argc >= 2)
    {
        try
        {
            n_target = std::stoi(argv[1]);
        }
        catch (...)
        {
            std::cerr << "Usage: " << argv[0] << " [n_target]\n";
            std::cerr << "  n_target: taille cible pour l'estimation (défaut: 24)\n";
            return 1;
        }
    }
    
    // Exécuter le benchmark complet avec affichage
    run_full_benchmark(test_n_values, n_target, 6, true);
    
    return 0;
}

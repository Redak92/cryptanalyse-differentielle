# Optimisations - Documentation Complète

## 📌 Vue d'ensemble

Ce document détaille les deux optimisations majeures implémentées pour la cryptanalyse différentielle du Toy Cipher :

1. **Points Distingués (Distinguished Points)** - Complexité O(2^n/2) sans saturation RAM
2. **Parallélisation Massive avec Threads** - Scalabilité sur architecture multi-cœur

---

## 1️⃣ Points Distingués (Distinguished Points)

### Fichiers
- `src/cryptanalysis/distinguished_points.h`
- `src/cryptanalysis/distinguished_points.cpp`

### Principe théorique

L'algorithme des **Points Distingués** est une méthode pour trouver des collisions avec une complexité de $O(2^{n/2})$ sans stocker tous les intermediates en RAM.

**Idée clé** : Au lieu de stocker chaque valeur, on ne mémorise que les valeurs qui satisfont une condition spécifique (les "points distingués").

### Configuration

```cpp
struct Config {
    uint32_t distinguishedBitCount = 16;  // k bits de poids faible = 0
    uint64_t maxWalkSteps = 10000;        // Itérations max par marche
    uint64_t maxMarches = 1000000;        // Nombre total de marches
    uint32_t numThreads = ...;             // Nombre de threads
    Difference targetDeltaIn = 0x0001;    // Différence d'entrée
};
```

### Algorithme

1. **Condition de point distingué** :
   ```cpp
   bool isDistinguished(Block value) {
       uint32_t mask = (1U << config.distinguishedBitCount) - 1;
       return (value & mask) == 0;  // k bits de poids faible = 0
   }
   ```

2. **Marche (Walk)** :
   - Partir d'un point `x0`
   - Itérer `x_{i+1} = G(x_i)` jusqu'à trouver un point distingué
   - Enregistrer `{valeur_distinguée, point_de_départ}`

3. **Détection de collision** :
   - Si deux marches différentes arrivent au même point distingué → **collision trouvée !**
   - Remonter les marches pour trouver les deux `x` qui donnent `G(x1) = G(x2)`

### Avantages

✅ Complexité O(2^n/2) garantie  
✅ Consommation RAM : O(nombre de points distingués) au lieu de O(2^n)  
✅ Parallélisable naturellement (marches indépendantes)  

### Exemple d'utilisation

```cpp
DistinguishedPoints::Config config;
config.distinguishedBitCount = 16;
config.maxMarches = 1000000;
config.numThreads = 8;

auto collisions = searcher.findCollisionsWithDistinguishedPoints(config);

for (const auto& collision : collisions) {
    std::cout << "x = " << Utils::toHex(collision.x) << "\n";
    std::cout << "y = " << Utils::toHex(collision.y) << "\n";
    std::cout << "deltaIn = " << Utils::toHex(collision.deltaIn) << "\n";
    std::cout << "deltaOut = " << Utils::toHex(collision.deltaOut) << "\n";
}
```

---

## 2️⃣ Parallélisation Massive

### Fichiers
- `src/cryptanalysis/parallel_search.h`
- `src/cryptanalysis/parallel_search.cpp`

### Stratégie

La parallélisation divise le travail entre plusieurs threads :

- **Division de l'espace de recherche** : Chaque thread traite un segment différent
- **Structures thread-safe** : Mutex pour la fusion des résultats
- **Optimisation cache** : `AlignedCounter` avec padding pour éviter le "false sharing"

### False Sharing - Optimisation critique

Le **faux partage** (false sharing) se produit quand :
- Deux threads modifient des variables proches en mémoire
- Elles se retrouvent dans la même ligne de cache (64 bytes)
- Chaque modification invalide le cache de l'autre thread → contention

**Solution** : Aligner chaque compteur sur une limite de 64 bytes (une ligne de cache)

```cpp
struct AlignedCounter {
    alignas(64) std::atomic<uint64_t> value{0};  // Chaque compteur = sa propre ligne
};
```

### Architecture

```
Main Thread
    ↓
spawner N worker threads
    ↓
┌─────────────────────────────────────────────┐
│ Thread 0  │ Thread 1  │ ... │ Thread N-1   │
│ Samples   │ Samples   │     │ Samples      │
│ 0..10000  │ 10000..   │     │ ...          │
└─────────────────────────────────────────────┘
    ↓         ↓         ↓     ↓
Buffer local (sans mutex)
    ↓
Fusion avec globalDifferentials (mutex une fois)
```

### Configuration

```cpp
struct Config {
    uint32_t numThreads = std::thread::hardware_concurrency();  // Auto-detect
    uint64_t samplesPerThread = 100000;
    Difference targetDeltaIn = 0x0001;
};
```

### Code du worker thread

```cpp
void workerThread(uint32_t threadId, ...) {
    std::unordered_map<Block, uint64_t> localBuffer;  // Sans verrou !
    
    // 1. Traitement local rapide (lecture/écriture sans mutex)
    for (uint64_t i = start; i < end; i++) {
        Block deltaOut = analyzeNewPair(i);
        localBuffer[deltaOut]++;
    }
    
    // 2. Fusion une seule fois (verrou court)
    {
        std::lock_guard<std::mutex> lock(globalMutex);
        for (const auto& entry : localBuffer) {
            globalDifferentials[entry.first] += entry.second;
        }
    }
}
```

### Performance

- **Speedup proche du linéaire** pour 4-8 threads (dépend de la contention)
- **Throughput** : Millions de samples/seconde (benchmark nécessaire pour valider)

### Exemple d'utilisation

```cpp
DifferentialCount results = searcher.searchDifferentialsParallel(
    0x0001,      // deltaIn
    8            // numThreads
);

searcher.printStatistics();  // Affiche throughput, etc.
```

---

## 3️⃣ Intégration dans DifferentialSearch

### API unifiée

```cpp
class DifferentialSearch {
    // Méthodes nouvelles
    DifferentialCount searchDifferentialsParallel(Difference deltaIn, uint32_t numThreads);
    std::vector<DistinguishedPoints::CollisionResult> 
        findCollisionsWithDistinguishedPoints(const DistinguishedPoints::Config& config);
};
```

### Initialisation

Les optimisations sont auto-initialisées dans le constructeur :

```cpp
DifferentialSearch::DifferentialSearch(ToyCipher& cipher, uint64_t maxSamples)
    : cipher(cipher), maxSamples(maxSamples) {
    
    // Configuration automatique de ParallelSearch
    ParallelSearch::Config psConfig;
    psConfig.numThreads = std::thread::hardware_concurrency();
    psConfig.samplesPerThread = maxSamples / psConfig.numThreads;
    parallelSearch = std::make_unique<ParallelSearch>(cipher, psConfig);
    
    // Configuration automatique de DistinguishedPoints
    DistinguishedPoints::Config dpConfig;
    dpConfig.numThreads = std::thread::hardware_concurrency();
    dpConfig.maxMarches = maxSamples;
    distinguishedPoints = std::make_unique<DistinguishedPoints>(cipher, dpConfig);
}
```

---

## 🎯 Guide d'utilisation complet

### Exemple 1 : Recherche basique non-optimisée
```cpp
DifferentialSearch searcher(cipher, 1000000);
DifferentialCount results = searcher.searchDifferentials(0x0001);
searcher.printStatistics();
```

### Exemple 2 : Recherche parallélisée
```cpp
DifferentialCount parallelResults = searcher.searchDifferentialsParallel(0x0001, 8);
// Même interface que searchDifferentials, mais plus rapide !
```

### Exemple 3 : Points Distingués (attaque complète)
```cpp
DistinguishedPoints::Config config;
config.distinguishedBitCount = 16;    // Points dont les 16 bits bas = 0
config.maxMarches = 1000000;
config.maxWalkSteps = 10000;
config.numThreads = 8;

auto collisions = searcher.findCollisionsWithDistinguishedPoints(config);

for (const auto& collision : collisions) {
    if (collision.found) {
        std::cout << "Collision : " << Utils::toHex(collision.x) 
                  << " -> " << Utils::toHex(collision.y) << "\n";
    }
}
```

### Exemple 4 : Pipeline complet multi-deltas

```cpp
std::vector<Difference> deltaIns = {0x0001, 0x0002, 0x0004, 0x0008};

for (const auto& deltaIn : deltaIns) {
    std::cout << "\n--- Analysing deltaIn = " << Utils::toHex(deltaIn) << " ---\n";
    searcher.searchDifferentialsParallel(deltaIn, 8);
}

auto best = searcher.findBestDifferentials(10, 0.001);  // Top 10
for (const auto& diff : best) {
    std::cout << "Prob: " << diff.second << "\n";
}
```

---

## 📊 Complexités et estimations

### Recherche basique
- **Complexité** : O(maxSamples × rounds_chiffre)
- **RAM** : O(nombre de différentielles uniques)
- **Temps** : ~0.1s pour 1M samples sur CPU modernePoint Distingué
- **Complexité** : O(2^n/2) where n = taille bloc
- **RAM** : O(√(2^n)) au lieu de O(2^n) ← 🔑 avantage clé
- **Speedup parallèle** : ~1.8-3x pour 4 threads, ~3-6x pour 8 threads

### Parallélisation
- **Overhead threads** : ~5-10% pour N threads (context switch)
- **Speedup idéal** : N sur N threads
- **Speedup réel** : ~0.7-0.9 × N (dépend contention)

---

## ⚙️ Configuration recommandée

| Scénario | Approche | Threads | Samples |
|----------|----------|---------|---------|
| **Test rapide** | Basique | N/A | 10k |
| **Analyse produc** | Parallèle | 8 | 1M |
| **Attaque complète** | Points Dist. | 16 | 10M+ |

---

## 🔧 Construction et compilation

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)
project(CryptanalyseDifferentielle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(differential_search ${SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(differential_search PRIVATE Threads::Threads)
```

### Compilation
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
./differential_search.exe
```

---

## 🧪 Validation

### Tests à effectuer
- [ ] Vérifier que les collisions trouvées sont exactes (G(x1) = G(x2))
- [ ] Comparer time parallèle vs basique
- [ ] Profiler cache misses avec `perf` ou equivalent
- [ ] Valider scalabilité avec 1, 2, 4, 8, 16 threads

### Benchmark suggéré
```cpp
auto start = std::chrono::high_resolution_clock::now();
auto results = searcher.searchDifferentialsParallel(0x0001, 8);
auto end = std::chrono::high_resolution_clock::now();
double elapsed = std::chrono::duration<double>(end - start).count();
std::cout << "Throughput : " << (1000000 / elapsed / 1e6) << " M samples/s\n";
```

---

## 📝 Notes de sécurité

- Les Points Distingués ne supposent pas connaissance de la clé ✓
- L'algorithme est purement statistique (pas de faux positifs)
- Adaptable à tout chiffre (pas limité à ToyCipher)


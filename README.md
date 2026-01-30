## Structure du Projet

```text
Crypto_TER/
├── report/                      # [Rapport] Les sources LaTeX du rapport.
│
├── src/
│   ├── interfaces/
│   │   └── ICipher.h            # [Interface] Classe abstraite définissant un chiffrement par bloc.
│   │
│   ├── ciphers/                 # [Classes] Les implémentations des chiffrements (Cibles).
│   │   │                        # ⚠️ CHAQUE DOSSIER DE CIPHER DOIT CONTENIR :
│   │   │                        #    - Le code source (.h/.cpp)
│   │   │                        #    - Une image/schéma de l'architecture du chiffrement
│   │   │                        #    - Un README.md affichant cette image
│   │   ├── ToySPN/              # Implémentation du SPN 16-bits (Baby-SPN).
│   │   ├── CustomFeistel/       # Implémentation du Feistel maison.
│   │   └── Speck/               # Implémentation de SPECK 32/64.
│   │
│   ├── analysis/                # [Classes] La logique de cryptanalyse.
│   │   └── DifferentialSearch.h # Classe principale contenant 3 algorithmes de recherche :
│   │                              1. Brute Force (Naïf absolu)
│   │                              2. Standard Optimisé (Naïf intelligent)
│   │                              3. Fondamental (Dinur et al. - Surrogate)
│   │
│   └── utils/                   # [Utilitaires]
│       ├── Types.h              # Définitions des types (Block, Key, Difference).
│       └── BitUtils.h/.cpp      # Fonctions statiques (rotations, affichage hex, random).
│
└── apps/                        # [Exécutables] Points d'entrée contenant les fonctions main().
    ├── demo_spn.cpp             # Lancement de l'attaque sur ToySPN.
    ├── demo_feistel.cpp         # Lancement de l'attaque sur le Feistel.
    └── demo_speck.cpp           # Reproduction des résultats sur SPECK.
```

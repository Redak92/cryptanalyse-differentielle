# Copilot Instructions - Differential Cryptanalysis Framework

## Project Overview

C++17 framework for differential cryptanalysis research on toy block ciphers. Educational TER project focusing on finding input/output differential pairs with exploitable probabilities.

## Architecture

```
ICipher (interface)          Analysis Engine
    │                             │
    ├── ToySPN (16-bit SPN)       ├── DifferentialSearch (3 algorithms)
    ├── CustomFeistel (12-bit)    └── naive_analysis (DDT exhaustive)
    └── Speck (32-bit ARX)
```

**Data flow**: Apps (`apps/`) instantiate a cipher → inject into analysis class → run differential search → output best (α, β, probability) tuples.

## Adding a New Cipher

1. Create folder under `src/ciphers/<CipherName>/` with `.h`, `.cpp`, and `README.md` (must include architecture diagram)
2. Implement `ICipher` interface (`src/interfaces/ICipher.h`):
   ```cpp
   Block encrypt(Block plaintext) const override;
   Block decrypt(Block ciphertext) const override;
   int getBlockSize() const override;  // Returns bit size (12, 16, 32, etc.)
   ```
3. Register in `CMakeLists.txt` under `crypto_core` library
4. Use types from `src/utils/Types.h`: `Block`, `Key`, `Difference` (all `uint64_t`)

## Analysis Algorithms

Located in `src/analysis/`:

| Class | Method | Complexity | Use Case |
|-------|--------|------------|----------|
| `DifferentialSearch` | `runBruteForceSearch()` | O(2^2n) | Only n ≤ 16 |
| `DifferentialSearch` | `runStandardSearch(k)` | O(2^n × k) | Random sampling |
| `DifferentialDistributionTable` | `compute_full_ddt_exhaustive()` | O(2^2n) | Full DDT |

**Key pattern**: Analysis classes take `const ICipher&` via constructor injection.

## Build & Run

```bash
mkdir build && cd build
cmake .. && make
./differential_analysis   # Main exhaustive analysis on CustomFeistel
./demo_spn                # ToySPN demo
./demo_speck              # Speck demo
```

## Project Conventions

- **Namespaces**: Feistel uses `diffcrypto::`, others use global (inconsistent—follow existing file patterns)
- **Round configuration**: Modify `NUM_FEISTEL_ROUNDS` constant in cipher headers, not hardcoded
- **S-boxes**: Define as `static constexpr` arrays in class definition (see `CustomFeistel.h`)
- **Include paths**: Use relative from `src/` (e.g., `#include "interfaces/ICipher.h"`)
- **Cipher interface**: All ciphers MUST implement `getBlockSize()` returning bit count (not bytes)

## Critical Files

- `src/analysis/naive_analysis.h` — `DifferentialDistributionTable` class and exhaustive DDT computation
- `src/analysis/DifferentialSearch.h` — Analysis algorithms using `const ICipher&` dependency injection
- `apps/main_differential.cpp` — Reference for running full analysis pipeline
- `src/ciphers/CustomFeistel/CustomFeistel.h` — Well-documented Feistel implementation example
## Testing & Validation

Since no test suite exists, validate changes by:
1. Rebuilding: `cd build && cmake .. && make`
2. Running relevant demo: `./demo_spn`, `./demo_speck`, or `./differential_analysis`
3. Comparing output to previous runs (look for consistent differential candidates)

## Known Limitations

- `ToySPN.cpp` is commented out in CMakeLists.txt (incomplete implementation)
- Block size limited to 64 bits (`uint64_t`) — impacts max cipher block size
- Namespace inconsistency between `diffcrypto::` (Feistel/naive_analysis) and global namespace (others)
- No test suite exists—validate manually via demo executables

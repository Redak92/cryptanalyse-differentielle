# Copilot Instructions - Differential Cryptanalysis Framework

## Project Overview

C++17 framework for differential cryptanalysis research on toy block ciphers (TER project). Finds input/output differential pairs (α, β) with exploitable probabilities through exhaustive DDT computation and optimization algorithms.

## Architecture & Data Flow

```
ICipher (interface)                Analysis Pipeline
    │                                   │
    ├── CustomFeistel (12-bit)     ├── DifferentialDistributionTable
    ├── Speck (32-bit ARX)         │   └── compute_full_ddt_exhaustive()
    ├── ToySPN (16-bit SPN)        ├── DifferentialSearch (3 algorithms)
    └── OTP (12-bit, trivial)      └── naive_analysis (helper functions)
```

**Workflow**: Cipher instance → passed to analysis class → exhaustive/sampling search → yields best differentials as `DifferentialPair` struct (α, β, probability).

## Adding a New Cipher

1. Create `src/ciphers/<Name>/` with `<Name>.h`, `.cpp`, `README.md` (include architecture diagram)
2. Inherit `ICipher`, implement:
   ```cpp
   Block encrypt(Block plaintext) const override;
   Block decrypt(Block ciphertext) const override;
   int getBlockSize() const override;  // Return bit size (not bytes)
   ```
3. Register `.cpp` in `CMakeLists.txt` `crypto_core` library source list
4. Use `src/utils/Types.h` types: `Block`, `Key`, `Difference` (all `uint64_t`)
5. **Bit masking**: Always mask final output to block size: `return (result & ((1ULL << BLOCK_BITS) - 1));`
6. Example: `src/ciphers/CustomFeistel/CustomFeistel.h` (see `NUM_FEISTEL_ROUNDS` config, S-box as `static constexpr`)

## Analysis Classes

**`DifferentialDistributionTable`** (`src/analysis/naive_analysis.h`):
- Stores 2D array of probabilities for all (Δx, Δy) pairs
- `compute_full_ddt_exhaustive()`: O(2^2n) — tests all plaintexts, exhausts all input differences
- `find_best_non_trivial()`: Returns single best (α, β, prob) excluding trivial (0→0)
- `find_best_non_trivial_top_n(n)`: Returns top N differentials ranked by probability

**`DifferentialSearch`** (`src/analysis/DifferentialSearch.h`):
- Takes `const ICipher&` in constructor (dependency injection pattern)
- `runBruteForceSearch()`: O(2^2n), test all plaintext pairs — only viable n ≤ 16
- `runStandardSearch(k)`: O(2^n × k), sample k pairs per input difference
- `runFundamentalAlgorithm(samples)`: Advanced collision-based method

## Build & Run

```powershell
# Windows (Visual Studio):h
mkdir build; cd build
cmake ..
cmake --build . --config Release

# Run:
.\Release\differential_analysis.exe
```

```bash
# Linux/macOS:
mkdir -p build && cd build
cmake .. && make -j4
./differential_analysis
```

**Executables**: `differential_analysis` (CustomFeistel, fully working), `demo_spn`/`demo_speck` (stubs)

## Project Conventions

- **Namespaces**: `diffcrypto::` used in Feistel & naive_analysis; global scope for others (inconsistent—preserve existing style)
- **Configuration**: Set `NUM_FEISTEL_ROUNDS` in cipher `.h` header, NOT hardcoded in methods
- **S-boxes**: Define as `static constexpr std::array<uint8_t, SIZE>` at class scope (see `CustomFeistel.h`)
- **Includes**: Relative from `src/` dir (e.g., `#include "interfaces/ICipher.h"`)
- **DDT Access**: Use `DifferentialDistributionTable::get_probability(Δx, Δy)` — handles 2D→1D indexing

## Critical Files for Reference

| File | Purpose |
|------|---------|
| `src/interfaces/ICipher.h` | Abstract cipher base; all ciphers inherit here |
| `src/analysis/naive_analysis.h` | `DifferentialDistributionTable` class, exhaustive DDT engine |
| `src/analysis/DifferentialSearch.h` | Three search algorithms; use injection pattern |
| `src/ciphers/CustomFeistel/CustomFeistel.h` | Reference cipher: S-box, key schedule, round structure |
| `apps/main_differential.cpp` | Example: instantiate cipher → DDT compute → find top-N → output |
| `src/utils/Types.h` | Type aliases: `Block`, `Key`, `Difference` |

## Testing & Validation

No formal test suite. Validate via:
1. **Rebuild**: `cmake --build . --config Release` (Windows) or `make` (Linux)
2. **Run** `differential_analysis` — should output best non-trivial differential with bias ≠ 0
3. **Compare** outputs across cipher config changes (e.g., `NUM_FEISTEL_ROUNDS`) — verify differentials remain consistent
4. **For new ciphers**: Stub main in `apps/demo_<name>.cpp`, run to verify no build errors

## Known Limitations & Inconsistencies

- `ToySPN` (commented in CMakeLists), `OTP` (not registered), `demo_spn.cpp`/`demo_feistel.cpp` incomplete stubs
- Block size capped at 64 bits (`uint64_t`) — max cipher block size
- Namespace inconsistency (`diffcrypto::` vs global) — unresolved, follow local file pattern
- DDT complexity O(2^2n) infeasible for n > 20; use sampling algorithms for larger ciphers
- No differential cache/memoization — each analysis recomputes full DDT

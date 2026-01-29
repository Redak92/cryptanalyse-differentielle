#ifndef PARALLEL_SEARCH_H
#define PARALLEL_SEARCH_H

#include "../utils/types.h"
#include "../cipher/toy_cipher.h"
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include <cstdint>

class ParallelSearch {
public:
    struct Config {
        uint32_t numThreads = std::thread::hardware_concurrency();
        uint64_t samplesPerThread = 100000;
        Difference targetDeltaIn = 0x0001;
    };

    struct AlignedCounter {
        alignas(64) std::atomic<uint64_t> value{0};
    };

    ParallelSearch(ToyCipher& cipher, const Config& config);
    ~ParallelSearch() = default;

    DifferentialCount searchDifferentialsParallel(Difference deltaIn);

    void analyzeMultipleDifferencesParallel(const std::vector<Difference>& deltaIns);

    void printStatistics() const;

private:
    ToyCipher& cipher;
    Config config;

    std::vector<std::unique_ptr<AlignedCounter>> alignedCounts;

    DifferentialCount globalDifferentials;
    mutable std::mutex globalMutex;

    struct PerformanceStats {
        std::atomic<uint64_t> totalSamples{0};
        std::atomic<uint64_t> totalDifferentials{0};
        std::atomic<double> timeElapsed{0.0};
        mutable std::mutex statsMutex;
    } perfStats;

    void workerThread(
        uint32_t threadId,
        Difference deltaIn,
        uint64_t samplesStart,
        uint64_t samplesEnd
    );

    Block functionF(Block right, uint32_t roundKey) const;
};

#endif

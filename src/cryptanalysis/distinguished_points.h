#ifndef DISTINGUISHED_POINTS_H
#define DISTINGUISHED_POINTS_H

#include "../utils/types.h"
#include "../cipher/toy_cipher.h"
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <memory>
#include <cstdint>

class DistinguishedPoints {
public:
    struct DistinguishedPoint {
        Block value;
        Block startingPoint;
        uint32_t walkLength;
        uint32_t threadId;
    };

    struct CollisionResult {
        Block x;
        Block y;
        Block collision;
        Difference deltaIn;
        Difference deltaOut;
        bool found;
    };

    struct Config {
        uint32_t distinguishedBitCount = 16;
        uint64_t maxWalkSteps = 10000;
        uint64_t maxMarches = 1000000;
        uint32_t numThreads = std::thread::hardware_concurrency();
        Difference targetDeltaIn = 0x0001;
    };

    DistinguishedPoints(ToyCipher& cipher, const Config& config);
    ~DistinguishedPoints() = default;

    std::vector<CollisionResult> findCollisions();

    void printStatistics() const;

    struct Stats {
        uint64_t totalWalks = 0;
        uint64_t collisionsFound = 0;
        uint64_t distinguishedPointsFound = 0;
        mutable std::mutex statsMutex;
    };

    const Stats& getStats() const { return stats; }

private:
    ToyCipher& cipher;
    Config config;

    std::unordered_map<Block, DistinguishedPoint> distinguishedTable;
    mutable std::mutex tableWriteMutex;

    Stats stats;

    bool isDistinguished(Block value) const;

    DistinguishedPoint performWalk(Block startingPoint);

    Block functionG(Block x) const;

    CollisionResult traceback(
        Block x1, Block y1,
        const DistinguishedPoint& x1Point,
        const DistinguishedPoint& y1Point
    );

    void workerThread(uint32_t threadId, uint64_t marcheStart, uint64_t marcheEnd);
};

#endif

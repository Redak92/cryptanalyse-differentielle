#ifndef DIFFERENTIAL_SEARCH_H
#define DIFFERENTIAL_SEARCH_H

#include "../utils/types.h"
#include "../cipher/toy_cipher.h"
#include "distinguished_points.h"
#include <vector>
#include <memory>
#include <thread>

class DifferentialSearch {
public:
    DifferentialSearch(ToyCipher& cipher, uint64_t maxSamples = 1000000);

    std::vector<DistinguishedPoints::CollisionResult> findCollisionsWithDistinguishedPoints(
        const DistinguishedPoints::Config& config);

    void printStatistics() const;

private:
    ToyCipher& cipher;
    uint64_t maxSamples;

    std::unique_ptr<DistinguishedPoints> distinguishedPoints;
};

#endif

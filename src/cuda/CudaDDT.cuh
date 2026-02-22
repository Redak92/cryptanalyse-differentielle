#pragma once

#include <cuda_runtime.h>
#include <cstdint>
#include <vector>

namespace diffcrypto {
    class DifferentialDistributionTable;
}

namespace diffcrypto {
namespace cuda {

void checkCudaError(cudaError_t code, const char* file, int line);

#define CUDA_CHECK(ans) { diffcrypto::cuda::checkCudaError((ans), __FILE__, __LINE__); }

bool initCudaDDT(
    const uint8_t* sbox,
    const uint8_t* round_keys,
    int num_rounds,
    int block_bits
);

void cleanupCudaDDT();

bool isCudaAvailable();

bool compute_ddt_cuda(DifferentialDistributionTable& ddt);

bool compute_single_difference_cuda(
    DifferentialDistributionTable& ddt,
    uint64_t delta_in
);

bool compute_ddt_range_cuda(
    DifferentialDistributionTable& ddt,
    uint64_t start_delta,
    uint64_t end_delta
);

void getCudaDeviceInfo(
    int device_id,
    char* name,
    int& compute_capability,
    size_t& global_mem_mb
);

size_t estimateDDTMemory(int block_bits);

}
}

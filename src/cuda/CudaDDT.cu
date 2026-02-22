#include "CudaDDT.cuh"
#include "CudaCiphers.cuh"
#include "../analysis/naive_analysis.h"

#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cstdio>
#include <cstring>

namespace diffcrypto {
namespace cuda {

static bool s_cuda_initialized = false;
static uint64_t* d_counts = nullptr;

void checkCudaError(cudaError_t code, const char* file, int line)
{
    if (code != cudaSuccess)
    {
        fprintf(stderr, "CUDA Error: %s at %s:%d\n", 
                cudaGetErrorString(code), file, line);
    }
}

__global__ void compute_ddt_kernel_2d(
    uint64_t* counts,
    uint32_t n_bits,
    uint64_t block_count)
{
    uint64_t x = static_cast<uint64_t>(blockIdx.x) * blockDim.x + threadIdx.x;
    uint64_t delta_in = static_cast<uint64_t>(blockIdx.y) + 1;
    
    if (x >= block_count || delta_in >= block_count)
        return;
    
    uint64_t c1 = feistel_encrypt_device(x);
    uint64_t c2 = feistel_encrypt_device(x ^ delta_in);
    uint64_t beta = c1 ^ c2;
    uint64_t idx = (delta_in << n_bits) | beta;
    
    atomicAdd(&counts[idx], 1ULL);
}

__global__ void compute_single_diff_kernel(
    uint64_t* counts,
    uint32_t n_bits,
    uint64_t block_count,
    uint64_t delta_in)
{
    uint64_t x = static_cast<uint64_t>(blockIdx.x) * blockDim.x + threadIdx.x;
    
    if (x >= block_count)
        return;
    
    uint64_t c1 = feistel_encrypt_device(x);
    uint64_t c2 = feistel_encrypt_device(x ^ delta_in);
    uint64_t beta = c1 ^ c2;
    uint64_t idx = (delta_in << n_bits) | beta;
    
    atomicAdd(&counts[idx], 1ULL);
}

bool initCudaDDT(
    const uint8_t* sbox,
    const uint8_t* round_keys,
    int num_rounds,
    int block_bits)
{
    int device_count = 0;
    CUDA_CHECK(cudaGetDeviceCount(&device_count));
    
    if (device_count == 0)
        return false;
    
    CUDA_CHECK(cudaMemcpyToSymbol(d_SBOX6, sbox, 64 * sizeof(uint8_t)));
    CUDA_CHECK(cudaMemcpyToSymbol(d_round_keys, round_keys, num_rounds * sizeof(uint8_t)));
    CUDA_CHECK(cudaMemcpyToSymbol(d_num_rounds, &num_rounds, sizeof(int)));
    CUDA_CHECK(cudaMemcpyToSymbol(d_block_bits, &block_bits, sizeof(int)));
    
    s_cuda_initialized = true;
    return true;
}

void cleanupCudaDDT()
{
    if (d_counts != nullptr)
    {
        cudaFree(d_counts);
        d_counts = nullptr;
    }
    s_cuda_initialized = false;
}

bool isCudaAvailable()
{
    int device_count = 0;
    cudaError_t err = cudaGetDeviceCount(&device_count);
    if (err != cudaSuccess || device_count == 0)
        return false;
    return true;
}

bool compute_ddt_cuda(DifferentialDistributionTable& ddt)
{
    if (!s_cuda_initialized)
        return false;
    
    uint32_t n = ddt.block_bits();
    uint64_t block_count = ddt.table_dimension();
    uint64_t table_size = 1ULL << (2 * n);
    
    size_t required_mem = table_size * sizeof(uint64_t);
    size_t free_mem, total_mem;
    CUDA_CHECK(cudaMemGetInfo(&free_mem, &total_mem));
    
    if (required_mem > free_mem * 0.9)
        return false;
    
    CUDA_CHECK(cudaMalloc(&d_counts, table_size * sizeof(uint64_t)));
    CUDA_CHECK(cudaMemset(d_counts, 0, table_size * sizeof(uint64_t)));
    
    const int threads_per_block = 256;
    
    dim3 blockDim(threads_per_block, 1, 1);
    dim3 gridDim(
        (block_count + threads_per_block - 1) / threads_per_block,
        block_count - 1,
        1
    );
    
    compute_ddt_kernel_2d<<<gridDim, blockDim>>>(d_counts, n, block_count);
    
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
    
    ddt.reset();
    CUDA_CHECK(cudaMemcpy(ddt.getCountsPtr(), d_counts, 
                          table_size * sizeof(uint64_t), 
                          cudaMemcpyDeviceToHost));
    
    CUDA_CHECK(cudaFree(d_counts));
    d_counts = nullptr;
    
    return true;
}

bool compute_single_difference_cuda(
    DifferentialDistributionTable& ddt,
    uint64_t delta_in)
{
    if (!s_cuda_initialized)
        return false;
    
    uint32_t n = ddt.block_bits();
    uint64_t block_count = ddt.table_dimension();
    uint64_t table_size = 1ULL << (2 * n);
    
    if (d_counts == nullptr)
    {
        CUDA_CHECK(cudaMalloc(&d_counts, table_size * sizeof(uint64_t)));
        CUDA_CHECK(cudaMemset(d_counts, 0, table_size * sizeof(uint64_t)));
    }
    
    const int threads_per_block = 256;
    int num_blocks = (block_count + threads_per_block - 1) / threads_per_block;
    
    compute_single_diff_kernel<<<num_blocks, threads_per_block>>>(
        d_counts, n, block_count, delta_in);
    
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());
    
    return true;
}

bool compute_ddt_range_cuda(
    DifferentialDistributionTable& ddt,
    uint64_t start_delta,
    uint64_t end_delta)
{
    for (uint64_t delta_in = start_delta; delta_in < end_delta; ++delta_in)
    {
        if (!compute_single_difference_cuda(ddt, delta_in))
            return false;
    }
    return true;
}

void getCudaDeviceInfo(
    int device_id,
    char* name,
    int& compute_capability,
    size_t& global_mem_mb)
{
    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, device_id));
    
    strcpy(name, prop.name);
    compute_capability = prop.major * 10 + prop.minor;
    global_mem_mb = prop.totalGlobalMem / (1024 * 1024);
}

size_t estimateDDTMemory(int block_bits)
{
    return (1ULL << (2 * block_bits)) * sizeof(uint64_t);
}

}
}

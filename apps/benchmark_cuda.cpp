#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <cmath>

#include "ciphers/CustomFeistel/CustomFeistel.h"
#include "analysis/naive_analysis.h"
#include "utils/Display.h"

#ifdef CUDA_ENABLED
#include "cuda/CudaDDT.cuh"
#endif

using namespace diffcrypto;
using Clock = std::chrono::high_resolution_clock;

double toMilliseconds(std::chrono::nanoseconds ns)
{
    return std::chrono::duration<double, std::milli>(ns).count();
}

DifferentialPair runCpuBenchmark(const CustomFeistel& cipher, double& elapsed_ms)
{
    uint32_t n_bits = cipher.getBlockSize();
    DifferentialDistributionTable ddt(n_bits);
    
    auto start = Clock::now();
        compute_full_ddt_exhaustive(cipher, ddt);
    ddt.normalize(ddt.table_dimension());
    
    auto end = Clock::now();
    elapsed_ms = toMilliseconds(end - start);
    
    return ddt.find_best_non_trivial();
}


#ifdef CUDA_ENABLED
DifferentialPair runGpuBenchmark(const CustomFeistel& cipher, double& elapsed_ms)
{
    uint32_t n_bits = cipher.getBlockSize();
    DifferentialDistributionTable ddt(n_bits);
    
    // Initialize CUDA with cipher parameters
    bool init_ok = cuda::initCudaDDT(
        CustomFeistel::getSBox(),
        cipher.getRoundKeys(),
        CustomFeistel::getNumRounds(),
        n_bits
    );
    
    if (!init_ok)
    {
        std::cerr << "Failed to initialize CUDA\n";
        elapsed_ms = -1;
        return {0, 0, 0.0};
    }
    
    auto start = Clock::now();
    

    bool compute_ok = cuda::compute_ddt_cuda(ddt);
    
    auto end = Clock::now();
    elapsed_ms = toMilliseconds(end - start);
    
    if (!compute_ok)
    {
        std::cerr << "GPU computation failed\n";
        cuda::cleanupCudaDDT();
        return {0, 0, 0.0};
    }
    

    ddt.normalize(ddt.table_dimension());
    
    cuda::cleanupCudaDDT();
    
    return ddt.find_best_non_trivial();
}
#endif


bool validateResults(const DifferentialPair& cpu, const DifferentialPair& gpu)
{
    const double epsilon = 1e-9;
    
    bool match = (cpu.delta_in == gpu.delta_in) &&
                 (cpu.delta_out == gpu.delta_out) &&
                 (std::abs(cpu.probability - gpu.probability) < epsilon);
    
    return match;
}

// ============================================================================
// Main
// ============================================================================

int main()
{
    printHeader("Differential Cryptanalysis - CPU vs GPU Benchmark");
    
    // Create cipher instance
    const uint16_t MASTER_KEY = 0xCAFE;
    CustomFeistel cipher(MASTER_KEY);
    
    std::cout << "\nCipher Configuration:\n";
    std::cout << "  - Type: CustomFeistel\n";
    std::cout << "  - Block size: " << cipher.getBlockSize() << " bits\n";
    std::cout << "  - Rounds: " << CustomFeistel::getNumRounds() << "\n";
    std::cout << "  - Master key: 0x" << std::hex << MASTER_KEY << std::dec << "\n";
    
    uint64_t block_count = 1ULL << cipher.getBlockSize();
    uint64_t total_encryptions = (block_count - 1) * block_count * 2;
    std::cout << "  - Total encryptions: " << total_encryptions << "\n\n";
    
    // ========================================================================
    // CPU Benchmark
    // ========================================================================
    
    printHeader("CPU Benchmark (Sequential)");
    
    double cpu_time_ms = 0;
    std::cout << "Running exhaustive DDT computation on CPU...\n";
    
    DifferentialPair cpu_result = runCpuBenchmark(cipher, cpu_time_ms);
    
    std::cout << "\nCPU Results:\n";
    std::cout << "  - Time: " << std::fixed << std::setprecision(2) << cpu_time_ms << " ms\n";
    std::cout << "  - Best differential: (0x" << std::hex << cpu_result.delta_in 
              << " -> 0x" << cpu_result.delta_out << std::dec << ")\n";
    std::cout << "  - Probability: " << std::fixed << std::setprecision(6) 
              << cpu_result.probability << "\n";
    std::cout << "  - Throughput: " << std::fixed << std::setprecision(2)
              << (total_encryptions / cpu_time_ms / 1000.0) << " M encryptions/sec\n";
    
    // ========================================================================
    // GPU Benchmark
    // ========================================================================
    
#ifdef CUDA_ENABLED
    printHeader("GPU Benchmark (CUDA Parallel)");
    
    // Check CUDA availability
    if (!cuda::isCudaAvailable())
    {
        std::cerr << "CUDA not available on this system.\n";
        return 1;
    }
    
    // Print device info
    char device_name[256];
    int compute_cap;
    size_t mem_mb;
    cuda::getCudaDeviceInfo(0, device_name, compute_cap, mem_mb);
    
    std::cout << "GPU Device:\n";
    std::cout << "  - Name: " << device_name << "\n";
    std::cout << "  - Compute Capability: " << (compute_cap / 10) << "." << (compute_cap % 10) << "\n";
    std::cout << "  - Global Memory: " << mem_mb << " MB\n";
    std::cout << "  - DDT Memory Required: " << cuda::estimateDDTMemory(cipher.getBlockSize()) / (1024 * 1024) << " MB\n\n";
    
    double gpu_time_ms = 0;
    std::cout << "Running exhaustive DDT computation on GPU...\n";
    
    DifferentialPair gpu_result = runGpuBenchmark(cipher, gpu_time_ms);
    
    if (gpu_time_ms > 0)
    {
        std::cout << "\nGPU Results:\n";
        std::cout << "  - Time: " << std::fixed << std::setprecision(2) << gpu_time_ms << " ms\n";
        std::cout << "  - Best differential: (0x" << std::hex << gpu_result.delta_in 
                  << " -> 0x" << gpu_result.delta_out << std::dec << ")\n";
        std::cout << "  - Probability: " << std::fixed << std::setprecision(6) 
                  << gpu_result.probability << "\n";
        std::cout << "  - Throughput: " << std::fixed << std::setprecision(2)
                  << (total_encryptions / gpu_time_ms / 1000.0) << " M encryptions/sec\n";
        
        // ====================================================================
        // Comparison
        // ====================================================================
        
        printHeader("Comparison Summary");
        
        bool results_match = validateResults(cpu_result, gpu_result);
        
        std::cout << "Validation: " << (results_match ? "PASSED" : "FAILED") << "\n";
        
        if (!results_match)
        {
            std::cerr << "WARNING: CPU and GPU results do not match!\n";
            std::cerr << "  CPU: delta_in=0x" << std::hex << cpu_result.delta_in 
                      << ", delta_out=0x" << cpu_result.delta_out 
                      << ", prob=" << std::dec << cpu_result.probability << "\n";
            std::cerr << "  GPU: delta_in=0x" << std::hex << gpu_result.delta_in 
                      << ", delta_out=0x" << gpu_result.delta_out 
                      << ", prob=" << std::dec << gpu_result.probability << "\n";
        }
        
        double speedup = cpu_time_ms / gpu_time_ms;
        std::cout << "\nPerformance:\n";
        std::cout << "  - CPU Time: " << std::fixed << std::setprecision(2) << cpu_time_ms << " ms\n";
        std::cout << "  - GPU Time: " << std::fixed << std::setprecision(2) << gpu_time_ms << " ms\n";
        std::cout << "  - Speedup:  " << std::fixed << std::setprecision(2) << speedup << "x\n";
        
        if (speedup > 1.0)
        {
            std::cout << "\n  GPU is " << std::fixed << std::setprecision(1) 
                      << speedup << "x FASTER than CPU\n";
        }
        else
        {
            std::cout << "\n  CPU is " << std::fixed << std::setprecision(1) 
                      << (1.0 / speedup) << "x faster (GPU overhead dominates for small blocks)\n";
        }
    }
    
#else
    printHeader("GPU Benchmark (CUDA)");
    std::cout << "CUDA support not compiled. Rebuild with CUDA Toolkit installed.\n";
#endif
    
    printSeparator();
    std::cout << "Benchmark complete.\n";
    
    return 0;
}

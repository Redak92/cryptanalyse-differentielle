# This file handles building each task as sub-project.
#
# Each task becomes an independent project in which you can experiment.
#
# Tasks are added as separate sub-projects so as compilation errors in
# one task don't affect another task.

cmake_minimum_required(VERSION 3.18)
project(cryptanalyse-differentielle LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

# CUDA Configuration
include(CheckLanguage)
check_language(CUDA)

if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    set(CMAKE_CUDA_ARCHITECTURES 86)  # RTX 3070 = SM 8.6 (Ampere)
    message(STATUS "CUDA enabled - Compiler: ${CMAKE_CUDA_COMPILER}")
    add_compile_definitions(CUDA_ENABLED)
else()
    message(WARNING "CUDA not found - GPU acceleration disabled")
endif()

# Configuration
include_directories(src)

# Librairie principale
add_library(crypto_core
        src/utils/BitUtils.cpp
        src/utils/DDTExport.cpp
        src/utils/Display.cpp
        src/analysis/DifferentialSearch.cpp
        src/analysis/naive_analysis.cpp
        src/ciphers/CustomFeistel/CustomFeistel.cpp
        src/ciphers/Speck/Speck.cpp
        src/ciphers/SPN/SPN.cpp
        src/ciphers/One-Time-Pad/otp.cpp
)

# Librairie CUDA (si disponible)
if(CMAKE_CUDA_COMPILER)
    add_library(crypto_cuda
        src/cuda/CudaDDT.cu
    )
    target_include_directories(crypto_cuda PUBLIC src/)
    set_target_properties(crypto_cuda PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
        CUDA_RESOLVE_DEVICE_SYMBOLS ON
    )
    target_link_libraries(crypto_core crypto_cuda)
endif()

# Exécutables
add_executable(demo_spn apps/demo_spn.cpp)
target_link_libraries(demo_spn crypto_core)

add_executable(differential_analysis apps/main_differential.cpp)
target_link_libraries(differential_analysis crypto_core)

add_executable(demo_speck apps/demo_speck.cpp)
target_link_libraries(demo_speck crypto_core)

add_executable(demo_otp apps/main_otp.cpp)
target_link_libraries(demo_otp crypto_core)

# Benchmark CUDA vs CPU
if(CMAKE_CUDA_COMPILER)
    add_executable(benchmark_cuda apps/benchmark_cuda.cpp)
    target_link_libraries(benchmark_cuda crypto_core crypto_cuda)
    set_target_properties(benchmark_cuda PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON
    )
else()
    # CPU-only version (CUDA disabled)
    add_executable(benchmark_cuda apps/benchmark_cuda.cpp)
    target_link_libraries(benchmark_cuda crypto_core)
endif()
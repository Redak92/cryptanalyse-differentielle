# This file handles building each task as sub-project.
#
# Each task becomes an independent project in which you can experiment.
#
# Tasks are added as separate sub-projects so as compilation errors in
# one task don't affect another task.

cmake_minimum_required(VERSION 3.10)
project(cryptanalyse-differentielle)

set(CMAKE_CXX_STANDARD 17)

# Configuration
include_directories(src)

# Librairie principale
add_library(crypto_core
        src/utils/BitUtils.cpp
        src/analysis/DifferentialSearch.cpp

        # src/ciphers/ToySPN/ToySPN.cpp
        # src/ciphers/Speck/Speck.cpp
        # src/ciphers/CustomFeistel/CustomFeistel.cpp
)

# Exécutables
add_executable(demo_spn apps/demo_spn.cpp)
target_link_libraries(demo_spn crypto_core)

# add_executable(demo_speck apps/demo_speck.cpp)
# target_link_libraries(demo_speck crypto_core)
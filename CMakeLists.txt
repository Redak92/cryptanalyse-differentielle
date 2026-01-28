cmake_minimum_required(VERSION 3.20)
project(CryptanalyseDifferentielle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options de compilation
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O3)
endif()

# Ajouter les fichiers sources
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Créer l'exécutable
add_executable(differential_search ${SOURCES})

# Inclure les répertoires
target_include_directories(differential_search PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Trouver les threads
find_package(Threads REQUIRED)
target_link_libraries(differential_search PRIVATE Threads::Threads)

# Afficher les fichiers compilés
message(STATUS "Sources trouvées: ${SOURCES}")

###############################################################################
# Code Graph RAG — C++ Implementation
# Root CMakeLists.txt
#
# HOW TO OPEN IN VISUAL STUDIO COMMUNITY:
#   1. Open Visual Studio → File → Open → CMake...
#   2. Navigate to THIS file (CMakeLists.txt)
#   3. Visual Studio will auto-generate the solution
#   4. Build → Build All (Ctrl+Shift+B)
#   5. Debug → Start Debugging (F5)
#
# HOW TO BUILD FROM COMMAND LINE:
#   mkdir build && cd build
#   cmake .. -G "Visual Studio 17 2022"
#   cmake --build . --config Debug
#
# DEPENDENCIES (install via vcpkg):
#   vcpkg install sqlite3 yaml-cpp nlohmann-json gtest
#
###############################################################################
cmake_minimum_required(VERSION 3.20)
project(CodeGraphRAG VERSION 1.0.0 LANGUAGES CXX)

# ── C++20 Standard ──────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Output directories ─────────────────────────────────────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ── Compiler warnings (learn from compiler errors!) ────────────────────────
if(MSVC)
    add_compile_options(/W4 /WX- /permissive- /utf-8)
    # /W4   = High warning level (great for learning)
    # /WX-  = Don't treat warnings as errors (so you can build while learning)
    # /permissive- = Strict C++ standards compliance
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ── Find Dependencies ──────────────────────────────────────────────────────
# TODO: Uncomment these as you install dependencies via vcpkg
# find_package(unofficial-sqlite3 CONFIG REQUIRED)
# find_package(yaml-cpp CONFIG REQUIRED)
# find_package(nlohmann_json CONFIG REQUIRED)

# ── Include directories ───────────────────────────────────────────────────
include_directories(${CMAKE_SOURCE_DIR}/src)

# ── Subdirectories (matches our architecture layers) ───────────────────────
add_subdirectory(src)
# add_subdirectory(tests)  # TODO: Uncomment when you add tests

# ── Main executable ───────────────────────────────────────────────────────
add_executable(code_graph_rag src/main.cpp)
target_link_libraries(code_graph_rag PRIVATE
    types_lib
    config_lib
    storage_lib
    # parsers_lib     # TODO: Uncomment as you build each layer
    agents_lib
    # semantic_lib
    # core_lib
)

message(STATUS "=== Code Graph RAG C++ Project Configured ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

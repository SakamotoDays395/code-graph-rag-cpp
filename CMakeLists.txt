cmake_minimum_required(VERSION 3.25)
project(CodeGraphRAG VERSION 1.0.0 LANGUAGES C CXX)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------
# 1. EXTERNAL DEPENDENCIES (Managed by vcpkg.json)
# ---------------------------------------------------------
# When you add a new library to vcpkg.json, you must find it here.
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# ---------------------------------------------------------
# 2. YOUR SOURCE FILES
# ---------------------------------------------------------
# Add all new .cpp files here. Do NOT use globbing.
add_executable(${PROJECT_NAME} 
    src/main.cpp
    # src/parsers/tree_sitter_parser.cpp <-- Example of how to add a file later
)

# Tell CMake where your header (.h) files live
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ---------------------------------------------------------
# 3. LINKING DEPENDENCIES
# ---------------------------------------------------------
# Link the external libraries to your executable
target_link_libraries(${PROJECT_NAME} PRIVATE 
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

# ---------------------------------------------------------
# 4. THIRD-PARTY (Tree-sitter)
# ---------------------------------------------------------
# When you are ready to integrate tree-sitter, uncomment the lines below 
# to build the grammars from the third_party/ folder and link them.

# add_subdirectory(third_party/tree-sitter-python)
# target_link_libraries(${PROJECT_NAME} PRIVATE tree-sitter-python)

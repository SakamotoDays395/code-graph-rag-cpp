# ════════════════════════════════════════════════════════════════════
#  CMakeLists.txt  ·  mini-code-graph
#  A teachable, extensible C++20 build graph for a single-language
#  code-graph MCP server. Written to grow — not to be rewritten.
# ════════════════════════════════════════════════════════════════════
# ───────────────────────────────────────────────────────────────────
# Lesson 0 — Block A: Project identity
# ───────────────────────────────────────────────────────────────────
# Explain: cmake_minimum_required gates the CMake *language version*.
#   3.25 is current enough to give us presets, CONFIG-mode find_package,
#   and $<TARGET_OBJECTS>. Visual Studio 18 2026 needs >=3.25, so we
#   match the generator floor — never higher than you actually need.
#
# Explain: project() declares the canonical project name used by every
#   downstream target as ${PROJECT_NAME}. We chose `mini_code_graph`
#   to distinguish it from the big repo's `code_graph_rag_mcp`.
#
# Explain: LANGUAGES C CXX — we need BOTH because tree-sitter grammars
#   are plain C (`parser.c`, `scanner.c`), while our code is C++20.
#   Forgetting C here is the #1 beginner CMake mistake with tree-sitter.
cmake_minimum_required(VERSION 3.25)
project(mini_code_graph
    VERSION     0.1.0
    LANGUAGES   C CXX
    DESCRIPTION "Mini code-graph MCP server"
)
# ───────────────────────────────────────────────────────────────────
# Lesson 0 — Block B: Language standard (Modern CMake idiom)
# ───────────────────────────────────────────────────────────────────
# Concept: set *properties on the target*, not on global variables.
#   Old CMake style used `set(CMAKE_CXX_STANDARD 20)` globally. Modern
#   style keeps the standard as a target property so two targets in the
#   same build can disagree. But for small projects the global form is
#   still the pragmatic default — it cascades to ALL targets below.
#
# Concept: CXX_EXTENSIONS OFF — forbids compiler-specific extensions
#   (e.g. `__int64` on MSVC, VLAs on GCC). Keeps code portable across
#   clang-cl, MSVC, and GCC. This is the *single most-skipped line*
#   in beginner CMake and it silently locks you to one compiler.
set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)
# Concept: export compile_commands.json so clangd / VSCode / CLion get
#   full IntelliSense. Without this, editors guess flags and fail at
#   `#include <nlohmann/json.hpp>` even when the build works.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# ───────────────────────────────────────────────────────────────────
# Lesson 0 — Block C: Dependency discovery (DIP applied to builds)
# ───────────────────────────────────────────────────────────────────
# SOLID tie-in — Dependency Inversion Principle (DIP):
#   "High-level modules should not depend on low-level modules; both
#    should depend on abstractions."
#
# In CMake terms: your code does NOT do `#include "C:/vcpkg/..."`.
#   It does `#include <nlohmann/json.hpp>` and asks CMake to *find*
#   an *abstract* target (`nlohmann_json::nlohmann_json`). vcpkg,
#   Conan, system packages — the *implementation* can change without
#   any source code touching. That's DIP in a build system.
#
# Concept: CONFIG mode means CMake uses the `.cmake` config file the
#   package ships (installed via vcpkg). Without CONFIG, CMake falls
#   back to *module mode* which probes around for headers — flaky.
#   Always prefer CONFIG for vcpkg-managed deps.
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog        CONFIG REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
# Concept: REQUIRED = build fails immediately if the package is missing.
#   Non-REQUIRED means "maybe the package exists" — that becomes a
#   runtime #ifdef nightmare. For core deps, never do that.
# ───────────────────────────────────────────────────────────────────
# Lesson 0 — Block D: Tree-sitter vendored grammar sources
# ───────────────────────────────────────────────────────────────────
# This is the *only* block that is mini-project-specific. The big repo
# supports 13 languages (battery of grammar .c files). We support ONE —
# C++. If you later add a second language, you add 2-4 lines here.
#
# Concept: cache variables (CACHE) let the user override a path from the
#   command line (`-DTREE_SITTER_VENDOR_INCLUDE_DIR=...`) without editing
#   this file. CMakePresets.json does exactly that for us, so end users
#   never need to know these paths exist.
set(TREE_SITTER_VENDOR_INCLUDE_DIR "" CACHE PATH
    "Directory containing tree_sitter/api.h")
set(TREE_SITTER_VENDOR_SOURCE_DIR  "" CACHE PATH
    "Directory containing tree-sitter lib.c (the core runtime)")
set(TREE_SITTER_GRAMMARS_ROOT      "" CACHE PATH
    "Directory containing tree-sitter-* grammar subfolders")
# Fallback probe — only fires if the user did not pass -D overrides.
#   Order matters: prefer in-repo third_party/ over external paths.
if(NOT TREE_SITTER_VENDOR_INCLUDE_DIR)
    foreach(_candidate
            "${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter/lib/include")
        if(EXISTS "${_candidate}/tree_sitter/api.h")
            set(TREE_SITTER_VENDOR_INCLUDE_DIR "${_candidate}")
            break()
        endif()
    endforeach()
endif()
if(NOT TREE_SITTER_VENDOR_SOURCE_DIR)
    foreach(_candidate
            "${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter/lib/src")
        if(EXISTS "${_candidate}/lib.c")
            set(TREE_SITTER_VENDOR_SOURCE_DIR "${_candidate}")
            break()
        endif()
    endforeach()
endif()
if(NOT TREE_SITTER_GRAMMARS_ROOT)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter-cpp/src/parser.c")
        set(TREE_SITTER_GRAMMARS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
    endif()
endif()
# Fail loudly if anything is missing. Silent failures here mean every
#   build later dies in opaque linker errors like "undefined symbol
#   ts_parser_new". Fail-fast beats fail-cryptic.
foreach(_var TREE_SITTER_VENDOR_INCLUDE_DIR TREE_SITTER_VENDOR_SOURCE_DIR TREE_SITTER_GRAMMARS_ROOT)
    if(NOT _var)
        message(FATAL_ERROR
            "Tree-sitter path not found: ${_var}. Set it via -D or edit CMakePresets.json.")
    endif()
endforeach()
# Concept: a "runtime" library — many small C files compiled together.
#   We compile the tree-sitter core ONCE (lib.c) plus each grammar's
#   parser.c/scanner.c into one static archive. Linking targets that
#   need tree-sitter pull the whole archive.
#
#   Why STATIC not SHARED? Because the entire project is one EXE.
#   Shared introduces DLL/SO export boilerplate you do not need
#   until you have *multiple* executables sharing the same code.
#   KISS until you outgrow KISS.
set(TREE_SITTER_RUNTIME_SOURCES
    "${TREE_SITTER_VENDOR_SOURCE_DIR}/lib.c"
    "${TREE_SITTER_GRAMMARS_ROOT}/tree-sitter-cpp/src/parser.c"
    "${TREE_SITTER_GRAMMARS_ROOT}/tree-sitter-cpp/src/scanner.c"
)
# Sanity check every source file exists *now* — better than a linker
#   error 30 seconds into the build.
foreach(_src IN LISTS TREE_SITTER_RUNTIME_SOURCES)
    if(NOT EXISTS "${_src}")
        message(FATAL_ERROR "Missing tree-sitter source file: ${_src}")
    endif()
endforeach()
add_library(tree_sitter_runtime STATIC ${TREE_SITTER_RUNTIME_SOURCES})
# Concept: target_include_directories(PUBLIC ...) makes the directory
#   visible to *both* this target AND anyone linking against it. That's
#   what we want — anyone who links tree_sitter_runtime must be able to
#   include <tree_sitter/api.h>.
target_include_directories(tree_sitter_runtime PUBLIC
        "${TREE_SITTER_VENDOR_INCLUDE_DIR}"
)
# Concept: tree-sitter grammars are honest ANSI C89. Some compilers
#   emit warnings on C89 with modern C++ flags. Set the language
#   standard per-target so the rest of our code stays C++20.
set_target_properties(tree_sitter_runtime PROPERTIES
    C_STANDARD          11
    C_STANDARD_REQUIRED ON
)
# MSVC bumped object-file section count past the default with many
#   tree-sitter grammars compiled into one static lib. Big repo hit
#   this exact bug (AGENTS.md decision tree-sitter-runtime step).
#   Cheap insurance even though we only have C++ grammar right now.
if(MSVC)
    target_compile_options(tree_sitter_runtime PRIVATE /bigobj)
endif()
# ───────────────────────────────────────────────────────────────────
# Lesson 0 — Block E: core_lib — your domain code (empty for now)
# ───────────────────────────────────────────────────────────────────
# ───────────────────────────────────────────────────────────────────
# Block E: core_lib — domain code (STATIC, grows per lesson)
# ───────────────────────────────────────────────────────────────────
# core_lib is a STATIC library. Source files are added via
# target_sources() as each lesson introduces new .cpp files.
# Lesson 1: types.hpp (header-only, no .cpp)
# Lesson 2: storage.cpp
# Lesson 3: parser.cpp
# Lesson 5: tools.cpp
# Lesson 6: main.cpp (in the executable, not core_lib)
add_library(core_lib STATIC)
set_target_properties(core_lib PROPERTIES LINKER_LANGUAGE CXX)
target_sources(core_lib PRIVATE
    src/storage.cpp
    src/parser.cpp
    src/tools.cpp
)
target_include_directories(core_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(core_lib
    PUBLIC
        spdlog::spdlog                  # exposed via logger.hpp
        nlohmann_json::nlohmann_json    # exposed via tools.hpp
    PRIVATE
        tree_sitter_runtime             # impl detail, never leaked out
        SQLiteCpp                       # impl detail, never leaked out
)
# ───────────────────────────────────────────────────────────────────
# Block F: The executable
# ───────────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE core_lib)

# ───────────────────────────────────────────────────────────────────
# Block G: Install rules (deferred by YAGNI)
# ───────────────────────────────────────────────────────────────────
# No install() yet. Add when CI / packaging becomes a real need.

# ════════════════════════════════════════════════════════════════════
#  SUMMARY
#    • core_lib is STATIC with sources: storage.cpp, parser.cpp, tools.cpp
#    • tree_sitter_runtime is STATIC, links the C++ grammar only.
#    • PUBLIC deps: spdlog, nlohmann_json (exposed via headers)
#    • PRIVATE deps: tree_sitter_runtime, SQLiteCpp (impl only)
#    • Compile-commands exported → clangd / IntelliSense works.
#    • Generator floor 3.25 matches VS 18 2026 / vcpkg presets.
# ════════════════════════════════════════════════════════════════════
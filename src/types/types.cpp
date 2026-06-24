///////////////////////////////////////////////////////////////////////////////
/// @file types.cpp
/// @brief Stub compilation unit for the types library
///
/// This file exists because CMake's add_library(STATIC ...) needs at least
/// one .cpp file. All type definitions are in headers (.h files), but we
/// need this to make the build system happy.
///////////////////////////////////////////////////////////////////////////////

// Include all type headers to verify they compile correctly
#include "entity_types.h"
#include "storage_types.h"
#include "parser_types.h"
#include "agent_types.h"
#include "semantic_types.h"
#include "error_types.h"

// This is intentionally empty — all types are defined in headers.

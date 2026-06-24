///////////////////////////////////////////////////////////////////////////////
/// @file storage_types.h
/// @brief Core data structures: Entity, Relationship, GraphQuery, GraphQueryResult
///
/// WHAT THIS FILE IS:
///   The "blueprints" for the 4 main data objects in the entire system.
///   Every other file in the project either CREATES, STORES, or QUERIES
///   these objects. This is the most important type file.
///
/// MAPS TO ORIGINAL: src/types/storage.ts (entire file)
///
/// RELATIONSHIP TO OTHER FILES:
///   - storage/graph_storage.cpp  → SAVES these to SQLite
///   - parsers/*                  → CREATES Entity objects from code
///   - agents/*                   → USES GraphQuery to search
///   - main.cpp                   → TESTS these in Milestone 1
///
/// WHAT TO LEARN:
///   - C++ structs: https://learncpp.com/cpp-tutorial/introduction-to-structs-members-and-member-selection/
///   - std::optional: https://en.cppreference.com/w/cpp/utility/optional
///   - std::unordered_map: https://en.cppreference.com/w/cpp/container/unordered_map
///   - std::variant: https://en.cppreference.com/w/cpp/utility/variant
///
/// DEBUGGING TIP:
///   In Visual Studio, create an Entity in main.cpp, set a breakpoint,
///   and expand the struct in the "Locals" window to see all fields.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <cstdint>

#include "entity_types.h"

namespace codegraph {

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// Position — A specific point in source code
//
// Used inside Location to mark where something starts and ends.
// Think of it like a cursor position in your text editor.
//
// Original TS:
//   location: {
//     start: { line: number; column: number; index: number };
//     end:   { line: number; column: number; index: number };
//   };
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct Position {
    int line   = 0;   // Line number (1-based, like your editor shows)
    int column = 0;   // Column number (0-based, characters from left)
    int index  = 0;   // Byte offset from start of file
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// Location — Where in the file an entity exists
//
// "The function `main()` starts at line 28, column 0 and ends at line 100"
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct Location {
    Position start;
    Position end;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// Entity — A "thing" found in source code (THE BUILDING)
//
// This is the MOST IMPORTANT struct in the entire project.
// Every function, class, variable, import found by the parser
// becomes an Entity object.
//
// Original TS (src/types/storage.ts:65-107):
//   export interface Entity {
//     id: string;
//     name: string;
//     type: EntityType;
//     ...
//   }
//
// WHO CREATES IT: Parsers (parsers/*.cpp)
// WHO STORES IT:  GraphStorage (storage/graph_storage.cpp)
// WHO QUERIES IT: Agents (agents/*.cpp), Tools (tools/*.cpp)
//
// EXERCISE: After writing this struct, go to main.cpp and create
//   an Entity object. Set a breakpoint and inspect it in the debugger.
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct Entity {
    std::string  id;            // Unique ID (SHA256 hash of content)
    std::string  name;          // Human-readable name ("calculateTax")
    EntityType   type;          // What kind (Function, Class, etc.)
    std::string  filePath;      // Which file it's in ("src/tax.cpp")
    Location     location;      // Where in the file (start/end positions)

    // Metadata — extra info that varies by entity type
    // In TypeScript this was: metadata?: { modifiers?: string[]; [key: string]: unknown; }
    // In C++, we use an unordered_map (hash map) for the dynamic key-value pairs
    std::unordered_map<std::string, std::string> metadata;

    std::string  hash;          // Content hash for change detection
    int64_t      createdAt = 0; // Unix timestamp in milliseconds
    int64_t      updatedAt = 0; // Last modification timestamp

    // TODO (Exercise): Add a method to print this entity for debugging:
    // std::string toString() const {
    //     return name + " (" + entityTypeToString(type) + ") in " + filePath;
    // }
};


// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// Relationship — A connection between two entities (THE ROAD)
//
// "Function A CALLS Function B"
// "Class Admin EXTENDS Class User"
// "File main.py IMPORTS module os"
//
// Original TS (src/types/storage.ts:112-127):
//   export interface Relationship {
//     id: string;
//     fromId: string;   ← The SOURCE entity
//     toId: string;     ← The TARGET entity
//     type: RelationType;
//     ...
//   }
//
// WHO CREATES IT: Parsers (when they detect function calls, imports, etc.)
// WHO STORES IT:  GraphStorage
// WHO QUERIES IT: Agents, to answer "what does X depend on?"
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct Relationship {
    std::string  id;            // Unique relationship ID
    std::string  fromId;        // Source entity's ID  (e.g., "calculateTotal")
    std::string  toId;          // Target entity's ID  (e.g., "calculateTax")
    RelationType type;          // What kind of link (Calls, Imports, Extends)

    // Optional metadata about this relationship
    std::optional<int>         line;      // Line where the call/import occurs
    std::optional<int>         column;    // Column position
    std::optional<std::string> context;   // Extra context string

    // Enhanced v2 fields
    std::optional<double>      weight;    // Relationship strength (1.0 = strong)
    std::optional<int64_t>     createdAt; // When this was detected
};


// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// FileInfo — Tracks which files have been indexed
//
// Before parsing a file again, the system checks: "Has this file changed
// since I last parsed it?" by comparing the hash.
//
// Original TS (src/types/storage.ts:132-137):
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct FileInfo {
    std::string path;           // Full file path
    std::string hash;           // SHA256 hash of file content
    int64_t     lastIndexed;    // Timestamp of last indexing
    int         entityCount;    // How many entities were found in this file
};


// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// GraphQuery — A SEARCH REQUEST (THE SEARCH BAR)
//
// When AI asks "Show me all classes in user.py", the system creates:
//   GraphQuery {
//     type = "entity",
//     filterEntityType = EntityType::Class,
//     filterFilePath = "user.py"
//   }
//
// This is then handed to GraphStorage.executeQuery() which translates
// it to SQL and returns a GraphQueryResult.
//
// Original TS (src/types/storage.ts:142-153)
//
// WHY std::optional: A filter might not be set. If filterEntityType
//   is std::nullopt, it means "don't filter by type, return all types."
//   This is equivalent to TypeScript's `entityType?: EntityType`.
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct GraphQuery {
    std::string type;    // "entity", "relationship", or "subgraph"

    // Filters (all optional — only apply the ones that are set)
    std::optional<EntityType>       filterEntityType;
    std::optional<RelationType>     filterRelationType;
    std::optional<std::string>      filterFilePath;
    std::optional<std::string>      filterName;        // Exact name match
    std::optional<std::string>      filterNamePattern; // Regex pattern

    // Graph traversal
    std::optional<int> depth;   // How many levels deep to traverse (BFS)

    // Pagination
    std::optional<int> limit;   // Max results to return
    std::optional<int> offset;  // Skip first N results

    // TODO (Advanced Exercise):
    // Add sorting: std::optional<std::string> orderBy;
    // Add direction: std::optional<std::string> orderDir; // "asc" or "desc"
};


// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// GraphQueryResult — THE ANSWER to a GraphQuery
//
// Remember: GraphQuery is the QUESTION, GraphQueryResult is the ANSWER.
//
// It contains:
//   1. entities[]        - The "buildings" found
//   2. relationships[]   - The "roads" connecting them
//   3. stats             - How long the search took (for performance monitoring)
//
// Original TS (src/types/storage.ts:158-166):
//   export interface GraphQueryResult {
//     entities: Entity[];
//     relationships: Relationship[];
//     stats: { totalEntities: number; totalRelationships: number; queryTimeMs: number; };
//   }
//
// WHO CREATES IT: GraphStorage.executeQuery()
// WHO USES IT:    Agents, Tools, MCP responses back to AI
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct QueryStats {
    int     totalEntities      = 0;
    int     totalRelationships = 0;
    double  queryTimeMs        = 0.0;
};

struct GraphQueryResult {
    std::vector<Entity>       entities;
    std::vector<Relationship> relationships;
    QueryStats                stats;
};


// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// EntityChange — For incremental updates (detect what changed)
//
// Instead of re-parsing the ENTIRE codebase when you edit one file,
// the system detects: "Only user.py changed. It has 2 new functions
// and 1 deleted function."
//
// Original TS (src/types/storage.ts:171-177):
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
enum class ChangeType {
    Added,
    Modified,
    Deleted
};

struct EntityChange {
    ChangeType                changeType;
    std::optional<Entity>     entity;     // The entity (if added/modified)
    std::optional<std::string> entityId;  // The entity ID (if deleted)
    std::string               filePath;   // Which file changed
    int64_t                   timestamp;  // When the change was detected
};


// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// Constants — Magic numbers given meaningful names
//
// Original TS (src/types/storage.ts:22-24):
//   export const MAX_BATCH_SIZE = 1000;
//   export const DEFAULT_CACHE_TTL = 300000; // 5 minutes
//   export const MAX_CONNECTIONS = 5;
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
constexpr int     MAX_BATCH_SIZE     = 1000;
constexpr int64_t DEFAULT_CACHE_TTL  = 300000;  // 5 minutes in milliseconds
constexpr int     MAX_CONNECTIONS    = 5;

} // namespace codegraph

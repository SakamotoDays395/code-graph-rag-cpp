///////////////////////////////////////////////////////////////////////////////
/// @file parser_types.h
/// @brief Types for the AST parsing layer
///
/// MAPS TO ORIGINAL: src/types/parser.ts
///
/// These types represent what the Parser EXTRACTS from source code before
/// it gets converted into Entity/Relationship objects for storage.
///
/// Flow: Source Code → Tree-Sitter → ParsedEntity → Entity (stored in DB)
///
/// WHAT TO LEARN:
///   - Recursive data structures (ParsedEntity has children: ParsedEntity[])
///   - std::vector as dynamic array
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>
#include <optional>

namespace codegraph {

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// Parameter — A function/method parameter
//
// Example: In `def add(a: int, b: int)`, there are 2 Parameters:
//   { name: "a", type: "int" } and { name: "b", type: "int" }
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct Parameter {
    std::string name;
    std::optional<std::string> type;         // Type annotation (may not exist)
    std::optional<std::string> defaultValue; // Default value (may not exist)
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ParsedEntity — Raw entity extracted from AST (before storing in DB)
//
// This is a TEMPORARY object. The parser creates it, then the indexer
// converts it into an Entity (the permanent version) and stores it.
//
// Note: This struct has a RECURSIVE field — `children` is a vector of
// ParsedEntity. A class contains methods, which are also ParsedEntities.
//
// Original TS (src/types/parser.ts)
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct ParsedEntity {
    std::string              name;
    std::string              type;        // "function", "class", etc.
    int                      startLine = 0;
    int                      endLine   = 0;
    int                      startColumn = 0;
    int                      endColumn   = 0;

    std::vector<std::string> modifiers;   // ["public", "static", "async"]
    std::vector<Parameter>   parameters;  // Function parameters
    std::vector<ParsedEntity> children;   // Methods inside a class (RECURSIVE!)
    std::vector<std::string> references;  // Other entities this one references
    std::vector<std::string> decorators;  // @decorator annotations

    std::optional<std::string> returnType;     // Function return type
    std::optional<std::string> documentation;  // Docstring / comment
    std::optional<std::string> parentClass;    // If this is a method, who owns it

    // TODO: Add these when you build the parser:
    // std::optional<std::string> sourceCode;  // The actual code text
    // std::vector<std::string> superClasses;  // For inheritance chains
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// ParseResult — What the parser returns after analyzing a file
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct ParsedRelationship {
    std::string fromName;
    std::string toName;
    std::string type;    // "calls", "imports", etc.
    int         line = 0;
};

struct ParseResult {
    std::string                    filePath;
    std::string                    language;     // "python", "cpp", etc.
    std::vector<ParsedEntity>      entities;
    std::vector<ParsedRelationship> relationships;
    double                         parseTimeMs = 0.0;

    // TODO: Add error tracking
    // std::vector<std::string> errors;
    // std::vector<std::string> warnings;
};

} // namespace codegraph

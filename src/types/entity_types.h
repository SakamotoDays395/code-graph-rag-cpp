///////////////////////////////////////////////////////////////////////////////
/// @file entity_types.h
/// @brief Enums for entity and relationship types in the code graph
///
/// WHAT THIS FILE IS:
///   This defines the VOCABULARY of our system. Just like a dictionary
///   defines what words exist, this file defines what KINDS of code
///   elements and connections our system can understand.
///
/// WHY ENUMS:
///   An enum is a "closed set" — it says "these are the ONLY valid values."
///   If someone tries to use an invalid type, the compiler catches it
///   at build time (not at runtime, which would be a crash).
///
/// MAPS TO ORIGINAL: src/types/storage.ts lines 33-60
///
/// WHAT TO LEARN:
///   - C++ enum class: https://en.cppreference.com/w/cpp/language/enum
///   - Why enum class > plain enum: prevents name collisions
///
/// TODO: Study the original TypeScript enums, then fill in any missing
///       values you want to support.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <unordered_map>

namespace codegraph {

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// EntityType — What KIND of code element is this?
//
// Original TypeScript (src/types/storage.ts:33-46):
//   export enum EntityType {
//     FUNCTION = "function",
//     CLASS = "class",
//     METHOD = "method",
//     ...
//   }
//
// In C++, we use `enum class` (scoped enum) instead of plain `enum`.
// Usage: EntityType::Function (not just Function — avoids naming conflicts)
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
enum class EntityType {
    Function,       // A standalone function (e.g., def calculate_tax(): ...)
    Class,          // A class definition (e.g., class UserLogin: ...)
    Method,         // A function INSIDE a class (e.g., UserLogin.validate())
    Interface,      // An interface/protocol (e.g., interface Printable { })
    Type,           // A type alias (e.g., type UserID = string)
    Document,       // A document/file entry
    Heading,        // A markdown heading
    Import,         // An import statement (e.g., import os)
    Export,         // An export statement (e.g., export default)
    Variable,       // A variable declaration (e.g., let x = 5)
    Constant,       // A constant (e.g., const PI = 3.14)
    Package,        // A package/module

    // TODO: Add more types as you add language analyzers
    // Struct,      // For C/C++ structs
    // Enum,        // For enum definitions
    // Trait,       // For Rust traits
    // Namespace,   // For C++ namespaces
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// RelationType — What KIND of connection exists between two entities?
//
// Think of these as verbs: Entity A [verb] Entity B
//   - "UserLogin" CALLS "validatePassword"
//   - "admin.py"  IMPORTS "user.py"
//   - "AdminUser" EXTENDS "User"
//
// Original TypeScript (src/types/storage.ts:51-60):
//   export enum RelationType {
//     CALLS = "calls",
//     IMPORTS = "imports",
//     ...
//   }
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
enum class RelationType {
    Calls,          // Function A calls Function B
    Imports,        // File A imports from File B
    Exports,        // File A exports Symbol B
    Extends,        // Class A inherits from Class B
    Implements,     // Class A implements Interface B
    References,     // Entity A references/uses Entity B
    Contains,       // File/Class A contains Function/Method B
    DependsOn,      // Module A depends on Module B

    // TODO: Add more as needed
    // Overrides,   // Method A overrides parent's Method B
    // Decorates,   // Decorator A decorates Function B
};


// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// Helper: Convert enum to string (for printing, debugging, database storage)
//
// WHY: Enums in C++ are just numbers internally. When you print them or
//      save them to SQLite, you need the string "function", not the number 0.
//
// USAGE:
//   EntityType t = EntityType::Function;
//   std::string s = entityTypeToString(t);  // Returns "function"
//
// DEBUGGING TIP: Set a breakpoint inside these functions and inspect
//   the `type` parameter to see how enums look in the debugger.
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
inline std::string entityTypeToString(EntityType type) {
    // TODO: Fill in the remaining cases to match the enum above
    switch (type) {
        case EntityType::Function:   return "function";
        case EntityType::Class:      return "class";
        case EntityType::Method:     return "method";
        case EntityType::Interface:  return "interface";
        case EntityType::Type:       return "type";
        case EntityType::Document:   return "document";
        case EntityType::Heading:    return "heading";
        case EntityType::Import:     return "import";
        case EntityType::Export:     return "export";
        case EntityType::Variable:   return "variable";
        case EntityType::Constant:   return "constant";
        case EntityType::Package:    return "package";
        default:                     return "unknown";
    }
}

inline std::string relationTypeToString(RelationType type) {
    switch (type) {
        case RelationType::Calls:       return "calls";
        case RelationType::Imports:     return "imports";
        case RelationType::Exports:     return "exports";
        case RelationType::Extends:     return "extends";
        case RelationType::Implements:  return "implements";
        case RelationType::References:  return "references";
        case RelationType::Contains:    return "contains";
        case RelationType::DependsOn:   return "depends_on";
        default:                        return "unknown";
    }
}

// TODO (Optional Advanced Exercise): Write the reverse functions:
//   EntityType stringToEntityType(const std::string& str);
//   RelationType stringToRelationType(const std::string& str);
// Hint: Use an unordered_map<string, EntityType> for O(1) lookup

} // namespace codegraph

#pragma once

#include <cstdint>
#include <string_view>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map> //reserved for Lesson 5 metadata bag.


namespace code_graph {

    struct Position {
        int line = 0;
        int column = 0;
        int index = 0;
    };

    struct Location {
        Position start;
        Position end;
    };
    
    enum class EntityType {
        Function,
        Class,
        Method,
        Struct,
        Variable,
        Field,
        Namespace,
        Enum,
        Typedef,
        Template
    };

    enum class RelationType {
        Calls,
        Contains,
        References,
        Extends,
        Implements,  // reserved — tree-sitter-cpp does not separate from Extends; not emitted in v1
        Uses,
        Imports
    };

    struct EntityMetadata {
        std::optional<std::vector<std::string>> modifiers;
        std::optional<std::string> returnType;
        std::optional<std::string> signature;
        std::optional<std::vector<std::string>> templateParams;
        std::optional<std::vector<std::string>> baseClasses;
    };

    struct Entity {
        std::string id;
        std::string name;
        EntityType type;
        std::string filePath;
        Location location;
        EntityMetadata metadata;
        std::string hash;
        std::int64_t createdAt = 0;
        std::int64_t updatedAt = 0;
    };

    struct RelationshipMetadata {
        std::optional<int> line;
        std::optional<std::string> context;
    };

    struct Relationship {
        std::string id;
        std::string fromId;
        std::string toId;
        RelationType type;
        std::optional<RelationshipMetadata> metadata;
        std::int64_t createdAt = 0;
    };

    struct FileInfo {
        std::string path;
        std::string hash;
        std::int64_t lastIndexed = 0;
        int entityCount = 0;
    };

    inline std::string_view entityTypeToString(EntityType type) {
        switch (type) {
            case EntityType::Function: return "function";
            case EntityType::Class: return "class";
            case EntityType::Method: return "method";
            case EntityType::Struct: return "struct";
            case EntityType::Variable: return "variable";
            case EntityType::Field: return "field";
            case EntityType::Namespace: return "namespace";
            case EntityType::Enum: return "enum";
            case EntityType::Typedef: return "typedef";
            case EntityType::Template: return "template";
        }
    }

    inline std::string_view relationTypeToString(RelationType type) {
        switch (type) {
            case RelationType::Calls: return "Calls";
            case RelationType::Contains: return "Contains";
            case RelationType::References: return "References";
            case RelationType::Extends: return "Extends";
            case RelationType::Implements: return "Implements";
            case RelationType::Uses: return "Uses";
            case RelationType::Imports: return "Imports";
        }
    }

}
#pragma once

#include <cstdint>
#include <string_view>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>


namespace codeGraph {

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
        FUNCTION,
        CLASS,
        INTERFACE,
        TYPE,
        DOCUMENT,
        HEADING,
        IMPORT,
        EXPORT,
        VARIABLE,
        CONSTANT,
        PACKAGE,
    };

    enum class RelationType {
        CALLS,
        IMPORTS,
        EXPORTS,
        EXTENDS,
        IMPLEMENTS,
        REFERENCES,
        CONTAINS,
        DEPENDS_ON,
    };

    struct EntityMetadata {
        std::optional<std::vector<std::string>> modifiers;
        std::optional<std::string> returnType;
        std::optional<std::string> signature;
        std::optional<std::string> language;
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
        std::optional<int> column;
        std::optional<std::string> context;
    };

    struct Relationship {
        std::string id;
        std::string fromId;
        std::string toId;
        RelationType type;
        std::optional<RelationshipMetadata> metadata;
        std::optional<int64_t> createdAt = 0;
    };

    struct FileInfo {
        std::string path;
        std::string hash;
        std::int64_t lastIndexed = 0;
        int entityCount = 0;
    };

    inline std::string_view entityTypeToString(EntityType type) {
        switch (type) {
            case EntityType::FUNCTION: return "FUNCTION";
            case EntityType::CLASS: return "CLASS";
            case EntityType::INTERFACE: return "INTERFACE";
            case EntityType::TYPE: return "TYPE";
            case EntityType::DOCUMENT: return "DOCUMENT";
            case EntityType::HEADING: return "HEADING";
            case EntityType::IMPORT: return "IMPORT";
            case EntityType::EXPORT: return "EXPORT";
            case EntityType::VARIABLE: return "VARIABLE";
            case EntityType::CONSTANT: return "CONSTANT";
            case EntityType::PACKAGE: return "PACKAGE";
        }
        return "unknown";
    }
    
}
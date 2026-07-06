#pragma once

#include "types.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace code_graph {

    struct EntityQuery {
        std::optional<EntityType> type;
        std::optional<std::string> filePath;
        std::optional<std::string> name;
        std::optional<int> limit = 100;
        std::optional<int> offset = 0;
    };

    class IGraphStorage {
        public:
        virtual ~IGraphStorage() = default;
        //Schema lifecycle 
        virtual void initialize() = 0;

        //Entity operations (CRUD)
        virtual std::string insertEntity(const Entity& entity) = 0;
        virtual std::optional<Entity> getEntity(const std::string& id) = 0;
        virtual std::vector<Entity> findEntities(const EntityQuery& query) = 0;
        virtual bool deleteEntity(const std::string& id) = 0;

        //Relationship CRUD
        virtual std::string insertRelationship(const Relationship& rel) = 0;
        virtual std::vector<Relationship> getRelationshipsForEntity(const std::string& entityId) = 0;

        //File tracking
        virtual void upsertFileInfo(const FileInfo& info) = 0;
        virtual std::optional<FileInfo> getFileInfo(const std::string& path) = 0;

        //stats 
        struct Stats {
            int totalEntities = 0;
            int totalRelationships = 0;
            int totalFiles = 0;
        };
        virtual Stats getStats() = 0;
    };

    std::unique_ptr<IGraphStorage> createSqliteStorage(const std::string& dbPath);

}
///////////////////////////////////////////////////////////////////////////////
/// @file batch_operations.h
/// @brief Bulk insert operations wrapped in a transaction
///
/// MAPS TO ORIGINAL: src/storage/batch-operations.ts
///
/// Instead of inserting 1000 entities one by one (1000 commits = slow),
/// this wraps them all in a single transaction (1 commit = fast).
///
/// WHAT TO LEARN: Database transactions (BEGIN/COMMIT/ROLLBACK)
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <vector>
#include "../types/storage_types.h"

namespace codegraph {

class GraphStorage;

class BatchOperations {
public:
    explicit BatchOperations(GraphStorage& storage);

    /// Insert many entities in a single transaction (fast!)
    /// Deduplicates by ID automatically
    void insertEntitiesBatch(const std::vector<Entity>& entities);

    /// Insert many relationships in a single transaction
    void insertRelationshipsBatch(const std::vector<Relationship>& relationships);

    /// Delete all entities for a file, then insert new ones (re-index)
    void replaceFileEntities(const std::string& filePath,
                             const std::vector<Entity>& entities,
                             const std::vector<Relationship>& relationships);

private:
    GraphStorage& storage_;
};

} // namespace codegraph

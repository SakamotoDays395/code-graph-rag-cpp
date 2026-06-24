///////////////////////////////////////////////////////////////////////////////
/// @file batch_operations.cpp
/// @brief Batch operations — IMPLEMENTATION
///////////////////////////////////////////////////////////////////////////////

#include "batch_operations.h"
#include "graph_storage.h"
#include <iostream>

namespace codegraph {

BatchOperations::BatchOperations(GraphStorage& storage) : storage_(storage) {}

void BatchOperations::insertEntitiesBatch(const std::vector<Entity>& entities) {
    // TODO: Wrap in a transaction for speed:
    // 1. Execute "BEGIN TRANSACTION"
    // 2. For each entity: storage_.insertEntity(entity)
    // 3. Execute "COMMIT"
    // 4. If any error: "ROLLBACK" (undo everything)
    //
    // WHY TRANSACTIONS: Without them, each INSERT does its own disk flush.
    //   With 1000 entities, that's 1000 disk flushes (slow!).
    //   With a transaction, it's just 1 disk flush (fast!).

    std::cout << "[BatchOps] Inserting " << entities.size() << " entities" << std::endl;
    for (const auto& entity : entities) {
        storage_.insertEntity(entity);
    }
}

void BatchOperations::insertRelationshipsBatch(const std::vector<Relationship>& relationships) {
    std::cout << "[BatchOps] Inserting " << relationships.size() << " relationships" << std::endl;
    for (const auto& rel : relationships) {
        storage_.insertRelationship(rel);
    }
}

void BatchOperations::replaceFileEntities(const std::string& filePath,
                                           const std::vector<Entity>& entities,
                                           const std::vector<Relationship>& relationships) {
    // TODO: In a single transaction:
    // 1. DELETE FROM entities WHERE file_path = ?
    // 2. INSERT all new entities
    // 3. INSERT all new relationships
    std::cout << "[BatchOps] Replacing entities in " << filePath << std::endl;
}

} // namespace codegraph

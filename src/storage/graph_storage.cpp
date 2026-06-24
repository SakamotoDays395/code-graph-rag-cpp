///////////////////////////////////////////////////////////////////////////////
/// @file graph_storage.cpp
/// @brief Graph storage — IMPLEMENTATION
///
/// TODO: This is the BIGGEST implementation challenge.
/// Implement each method one at a time, test with the debugger after each.
///
/// SUGGESTED ORDER:
///   1. Constructor + initialize()
///   2. insertEntity() + getEntity()   ← Test with Milestone 3 in main.cpp
///   3. insertRelationship() + getRelationshipsFrom()
///   4. executeQuery()                  ← Most complex, do last
///   5. getSubgraph()                   ← Requires BFS algorithm knowledge
///
/// DEBUGGING EXERCISE:
///   After implementing insertEntity():
///   1. Set breakpoint at insertEntity()
///   2. Create an entity in main.cpp and call insertEntity()
///   3. Step through (F10) to see the SQL being built
///   4. Open the .db file in "DB Browser for SQLite" to verify the row exists
///////////////////////////////////////////////////////////////////////////////

#include "graph_storage.h"
#include "sqlite_manager.h"
#include "schema_migrations.h"
#include "../types/error_types.h"

#include <iostream>
#include <sstream>
#include <chrono>
// #include <openssl/sha.h>  // TODO: For SHA256. Or use a simpler hash library.

namespace codegraph {

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
GraphStorage::GraphStorage(const std::string& dbPath)
    : sqliteManager_(std::make_unique<SqliteManager>(dbPath))
    , migrations_(std::make_unique<SchemaMigrations>(*sqliteManager_)) {
}

GraphStorage::~GraphStorage() = default;

void GraphStorage::initialize() {
    sqliteManager_->initialize();
    migrations_->runMigrations();
    std::cout << "[GraphStorage] Initialized. Entities: " << getEntityCount()
              << ", Relationships: " << getRelationshipCount() << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// Entity CRUD
// ─────────────────────────────────────────────────────────────────────────────

void GraphStorage::insertEntity(const Entity& entity) {
    // TODO: Execute this SQL:
    // INSERT INTO entities (id, name, type, file_path, location, metadata, hash, created_at, updated_at)
    // VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    // ON CONFLICT(id) DO UPDATE SET
    //   name = excluded.name,
    //   metadata = excluded.metadata,
    //   hash = excluded.hash,
    //   updated_at = excluded.updated_at;
    //
    // IMPORTANT: Use PREPARED STATEMENTS, not string concatenation!
    // String concatenation is vulnerable to SQL injection attacks.
    //
    // Steps:
    // 1. sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    // 2. sqlite3_bind_text(stmt, 1, entity.id.c_str(), -1, SQLITE_TRANSIENT);
    // 3. sqlite3_bind_text(stmt, 2, entity.name.c_str(), -1, SQLITE_TRANSIENT);
    // 4. ... bind all other fields
    // 5. sqlite3_step(stmt);
    // 6. sqlite3_finalize(stmt);

    std::cout << "[GraphStorage] insertEntity: " << entity.name
              << " (" << entityTypeToString(entity.type) << ")" << std::endl;
}

std::optional<Entity> GraphStorage::getEntity(const std::string& id) const {
    // TODO: Execute:
    // SELECT * FROM entities WHERE id = ?
    //
    // If a row is found, construct an Entity from the columns and return it.
    // If no row, return std::nullopt.

    std::cout << "[GraphStorage] getEntity: " << id << " (placeholder)" << std::endl;
    return std::nullopt;
}

std::vector<Entity> GraphStorage::getEntitiesByFile(const std::string& filePath) const {
    // TODO: SELECT * FROM entities WHERE file_path = ?
    std::cout << "[GraphStorage] getEntitiesByFile: " << filePath << std::endl;
    return {};
}

std::vector<Entity> GraphStorage::getEntitiesByType(EntityType type) const {
    // TODO: SELECT * FROM entities WHERE type = ?
    std::cout << "[GraphStorage] getEntitiesByType: " << entityTypeToString(type) << std::endl;
    return {};
}

void GraphStorage::deleteEntity(const std::string& id) {
    // TODO: DELETE FROM entities WHERE id = ?
    // Note: ON DELETE CASCADE in the schema will auto-delete relationships
    std::cout << "[GraphStorage] deleteEntity: " << id << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// Relationship CRUD
// ─────────────────────────────────────────────────────────────────────────────

void GraphStorage::insertRelationship(const Relationship& rel) {
    // TODO: INSERT INTO relationships (id, from_id, to_id, type, metadata, weight, created_at)
    //       VALUES (?, ?, ?, ?, ?, ?, ?)
    //       ON CONFLICT(id) DO UPDATE SET ...

    std::cout << "[GraphStorage] insertRelationship: "
              << rel.fromId << " --" << relationTypeToString(rel.type)
              << "--> " << rel.toId << std::endl;
}

std::vector<Relationship> GraphStorage::getRelationshipsFrom(const std::string& entityId) const {
    // TODO: SELECT * FROM relationships WHERE from_id = ?
    return {};
}

std::vector<Relationship> GraphStorage::getRelationshipsTo(const std::string& entityId) const {
    // TODO: SELECT * FROM relationships WHERE to_id = ?
    return {};
}

// ─────────────────────────────────────────────────────────────────────────────
// Query Operations (Most Complex!)
// ─────────────────────────────────────────────────────────────────────────────

GraphQueryResult GraphStorage::executeQuery(const GraphQuery& query) const {
    // TODO: This is the HARDEST method. It builds SQL dynamically from the query.
    //
    // Algorithm:
    // 1. Start with a base query: "SELECT * FROM entities WHERE 1=1"
    // 2. For each filter that is set (not nullopt):
    //    - If filterEntityType: append " AND type = ?"
    //    - If filterFilePath:   append " AND file_path = ?"
    //    - If filterName:       append " AND name = ?"
    // 3. Add limit/offset: " LIMIT ? OFFSET ?"
    // 4. Execute the query
    // 5. If query.type == "subgraph", also fetch relationships
    // 6. Measure queryTimeMs
    // 7. Return GraphQueryResult

    auto startTime = std::chrono::high_resolution_clock::now();

    GraphQueryResult result;

    // TODO: Build and execute dynamic SQL query here

    auto endTime = std::chrono::high_resolution_clock::now();
    result.stats.queryTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    std::cout << "[GraphStorage] executeQuery completed in "
              << result.stats.queryTimeMs << "ms" << std::endl;

    return result;
}

GraphQueryResult GraphStorage::getSubgraph(const std::string& entityId, int depth) const {
    // TODO: Use BFS (Breadth-First Search) to find all connected entities
    //
    // Algorithm:
    // 1. Start with a queue containing entityId
    // 2. For each entity in the queue:
    //    a. Get all relationships from/to this entity
    //    b. Add the connected entity IDs to the queue (if not visited)
    //    c. Add entity + relationships to the result
    // 3. Repeat until depth is reached or queue is empty
    //
    // Data structures needed:
    //   std::queue<std::string> bfsQueue;
    //   std::unordered_set<std::string> visited;
    //
    // LEARN BFS: https://www.youtube.com/watch?v=HZ5YTanv5QE

    std::cout << "[GraphStorage] getSubgraph: " << entityId
              << ", depth=" << depth << std::endl;

    GraphQueryResult result;
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// File Tracking
// ─────────────────────────────────────────────────────────────────────────────

void GraphStorage::updateFileInfo(const FileInfo& info) {
    // TODO: INSERT OR REPLACE INTO files (path, hash, last_indexed, entity_count)
    //       VALUES (?, ?, ?, ?)
    std::cout << "[GraphStorage] updateFileInfo: " << info.path << std::endl;
}

bool GraphStorage::isFileStale(const std::string& filePath, const std::string& currentHash) const {
    // TODO: SELECT hash FROM files WHERE path = ?
    //       If stored hash != currentHash, file is stale (needs re-indexing)
    return true;  // Default: always re-index
}

// ─────────────────────────────────────────────────────────────────────────────
// Stats
// ─────────────────────────────────────────────────────────────────────────────

int GraphStorage::getEntityCount() const {
    // TODO: SELECT COUNT(*) FROM entities
    return 0;
}

int GraphStorage::getRelationshipCount() const {
    // TODO: SELECT COUNT(*) FROM relationships
    return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// ID Generation
// ─────────────────────────────────────────────────────────────────────────────

std::string GraphStorage::generateEntityId(const std::string& name,
                                            const std::string& filePath,
                                            const std::string& type) const {
    // TODO: Generate a SHA256 hash of (name + filePath + type)
    // This ensures the same entity always gets the same ID.
    //
    // For now, use a simple concatenation as placeholder:
    return name + "::" + filePath + "::" + type;

    // Later, replace with:
    // #include <openssl/sha.h>
    // std::string input = name + "|" + filePath + "|" + type;
    // unsigned char hash[SHA256_DIGEST_LENGTH];
    // SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    // ... convert to hex string
}

} // namespace codegraph

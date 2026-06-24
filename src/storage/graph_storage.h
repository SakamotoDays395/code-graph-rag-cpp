///////////////////////////////////////////////////////////////////////////////
/// @file graph_storage.h
/// @brief Graph database CRUD operations (Facade Pattern)
///
/// MAPS TO ORIGINAL: src/storage/graph-storage.ts
///
/// This is the MAIN class that the rest of the system talks to.
/// It hides all the complex SQL behind simple methods like:
///   insertEntity(), getEntity(), executeQuery()
///
/// PATTERN USED: Facade — provides a simplified interface to complex subsystem
///
/// BUILD PHASE: 3 (the most important file in this phase)
///
/// WHAT TO LEARN:
///   - Facade Pattern: https://refactoring.guru/design-patterns/facade
///   - SQL INSERT/SELECT: https://www.sqlitetutorial.net/
///   - SHA-256 hashing: for generating stable entity IDs
///   - BFS algorithm: for getSubgraph()
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>

#include "../types/storage_types.h"

namespace codegraph {

class SqliteManager;
class SchemaMigrations;

class GraphStorage {
public:
    /// Constructor — takes the path to the SQLite database
    explicit GraphStorage(const std::string& dbPath);
    ~GraphStorage();

    /// Initialize the database (create tables, run migrations)
    void initialize();

    // ── CRUD: Entity Operations ────────────────────────────────────────
    
    /// Insert or update an entity (upsert)
    /// Uses SHA256 hash for stable ID generation
    void insertEntity(const Entity& entity);

    /// Get an entity by its ID
    /// Returns std::nullopt if not found
    std::optional<Entity> getEntity(const std::string& id) const;

    /// Get all entities in a specific file
    std::vector<Entity> getEntitiesByFile(const std::string& filePath) const;

    /// Get all entities of a specific type
    std::vector<Entity> getEntitiesByType(EntityType type) const;

    /// Delete an entity by ID
    void deleteEntity(const std::string& id);

    // ── CRUD: Relationship Operations ─────────────────────────────────

    /// Insert or update a relationship
    void insertRelationship(const Relationship& rel);

    /// Get all relationships from a specific entity
    std::vector<Relationship> getRelationshipsFrom(const std::string& entityId) const;

    /// Get all relationships to a specific entity (reverse lookup)
    std::vector<Relationship> getRelationshipsTo(const std::string& entityId) const;

    // ── Query Operations ──────────────────────────────────────────────

    /// Execute a GraphQuery and return results
    /// This is the main method the Agent system calls
    GraphQueryResult executeQuery(const GraphQuery& query) const;

    /// Get a subgraph centered on an entity, up to N levels deep (BFS)
    /// This is used when AI asks "show me everything connected to X"
    GraphQueryResult getSubgraph(const std::string& entityId, int depth) const;

    // ── File Tracking ─────────────────────────────────────────────────

    /// Track that a file has been indexed
    void updateFileInfo(const FileInfo& info);

    /// Check if a file needs re-indexing (hash changed)
    bool isFileStale(const std::string& filePath, const std::string& currentHash) const;

    // ── Stats ─────────────────────────────────────────────────────────
    int getEntityCount() const;
    int getRelationshipCount() const;

private:
    std::unique_ptr<SqliteManager>    sqliteManager_;
    std::unique_ptr<SchemaMigrations> migrations_;

    /// Generate a stable ID from entity content using SHA256
    /// Same content → same ID (idempotent)
    std::string generateEntityId(const std::string& name,
                                 const std::string& filePath,
                                 const std::string& type) const;
};

} // namespace codegraph

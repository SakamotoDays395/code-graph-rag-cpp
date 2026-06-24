///////////////////////////////////////////////////////////////////////////////
/// @file schema_migrations.h
/// @brief Database schema versioning and migrations
///
/// MAPS TO ORIGINAL: src/storage/schema-migrations.ts
///
/// Manages database table creation and upgrades. When you change the
/// schema (add a column, create a new table), you add a new migration
/// version here instead of editing old SQL.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>

namespace codegraph {

class SqliteManager;  // Forward declaration

class SchemaMigrations {
public:
    explicit SchemaMigrations(SqliteManager& db);

    /// Run all pending migrations
    void runMigrations();

    /// Get current schema version
    int getCurrentVersion() const;

private:
    SqliteManager& db_;

    /// Each migration is a version number + SQL to execute
    struct Migration {
        int         version;
        std::string description;
        std::string sql;
    };

    /// Define all migrations here
    std::vector<Migration> getAllMigrations() const;
};

} // namespace codegraph

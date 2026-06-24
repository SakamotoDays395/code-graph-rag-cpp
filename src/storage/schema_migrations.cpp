///////////////////////////////////////////////////////////////////////////////
/// @file schema_migrations.cpp
/// @brief Schema migrations — IMPLEMENTATION
///
/// TODO: Implement the migration runner. Each migration runs exactly once.
///
/// The migrations table tracks which versions have been applied:
///   CREATE TABLE migrations (version INTEGER PRIMARY KEY, applied_at INTEGER);
///////////////////////////////////////////////////////////////////////////////

#include "schema_migrations.h"
#include "sqlite_manager.h"
#include <iostream>

namespace codegraph {

SchemaMigrations::SchemaMigrations(SqliteManager& db) : db_(db) {}

std::vector<SchemaMigrations::Migration> SchemaMigrations::getAllMigrations() const {
    return {
        // ── Version 1: Initial schema ────────────────────────────────
        {1, "Create initial tables", R"SQL(
            CREATE TABLE IF NOT EXISTS entities (
                id          TEXT PRIMARY KEY,
                name        TEXT NOT NULL,
                type        TEXT NOT NULL,
                file_path   TEXT NOT NULL,
                location    TEXT,          -- JSON string: {"start":{"line":1}, "end":{"line":10}}
                metadata    TEXT,          -- JSON string: {"modifiers":["public","static"]}
                hash        TEXT NOT NULL,
                created_at  INTEGER NOT NULL,
                updated_at  INTEGER NOT NULL
            );

            CREATE TABLE IF NOT EXISTS relationships (
                id          TEXT PRIMARY KEY,
                from_id     TEXT NOT NULL,
                to_id       TEXT NOT NULL,
                type        TEXT NOT NULL,
                metadata    TEXT,
                weight      REAL DEFAULT 1.0,
                created_at  INTEGER,
                FOREIGN KEY (from_id) REFERENCES entities(id) ON DELETE CASCADE,
                FOREIGN KEY (to_id)   REFERENCES entities(id) ON DELETE CASCADE
            );

            CREATE TABLE IF NOT EXISTS files (
                path          TEXT PRIMARY KEY,
                hash          TEXT NOT NULL,
                last_indexed  INTEGER NOT NULL,
                entity_count  INTEGER DEFAULT 0
            );

            CREATE TABLE IF NOT EXISTS migrations (
                version     INTEGER PRIMARY KEY,
                applied_at  INTEGER NOT NULL
            );
        )SQL"},

        // ── Version 2: Add indexes for performance ──────────────────
        {2, "Add performance indexes", R"SQL(
            CREATE INDEX IF NOT EXISTS idx_entities_type      ON entities(type);
            CREATE INDEX IF NOT EXISTS idx_entities_file_path ON entities(file_path);
            CREATE INDEX IF NOT EXISTS idx_entities_name      ON entities(name);
            CREATE INDEX IF NOT EXISTS idx_rel_from_id        ON relationships(from_id);
            CREATE INDEX IF NOT EXISTS idx_rel_to_id          ON relationships(to_id);
            CREATE INDEX IF NOT EXISTS idx_rel_type           ON relationships(type);
        )SQL"},

        // TODO: Add Version 3 when you need the performance_metrics table
        // {3, "Add performance metrics table", R"SQL(
        //     CREATE TABLE IF NOT EXISTS performance_metrics (
        //         id         INTEGER PRIMARY KEY AUTOINCREMENT,
        //         operation  TEXT NOT NULL,
        //         duration   REAL NOT NULL,
        //         timestamp  INTEGER NOT NULL
        //     );
        // )SQL"},
    };
}

void SchemaMigrations::runMigrations() {
    // TODO: Implement this:
    // 1. Ensure the migrations table exists
    // 2. Get the current version (SELECT MAX(version) FROM migrations)
    // 3. For each migration where version > currentVersion:
    //    a. Execute the SQL
    //    b. INSERT INTO migrations (version, applied_at) VALUES (?, now)
    //    c. Log: "Applied migration v{version}: {description}"

    auto migrations = getAllMigrations();
    int currentVersion = getCurrentVersion();

    std::cout << "[Migrations] Current schema version: " << currentVersion << std::endl;
    std::cout << "[Migrations] Available migrations: " << migrations.size() << std::endl;

    for (const auto& m : migrations) {
        if (m.version > currentVersion) {
            // TODO: Execute m.sql on the database
            std::cout << "[Migrations] Applied v" << m.version
                      << ": " << m.description << std::endl;
        }
    }
}

int SchemaMigrations::getCurrentVersion() const {
    // TODO: Query the migrations table for MAX(version)
    // Return 0 if the table doesn't exist yet
    return 0;
}

} // namespace codegraph

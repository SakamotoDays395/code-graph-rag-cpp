///////////////////////////////////////////////////////////////////////////////
/// @file sqlite_manager.cpp
/// @brief SQLite connection manager — IMPLEMENTATION
///
/// TODO: Fill in each method. Start with the constructor and initialize().
///
/// DEBUGGING EXERCISE:
///   1. Set breakpoints in the constructor and initialize()
///   2. Step through (F10) to see the database being created
///   3. Check the "data/" folder — a .db file should appear
///////////////////////////////////////////////////////////////////////////////

#include "sqlite_manager.h"
#include "../types/error_types.h"
#include <iostream>
#include <filesystem>

// TODO: Uncomment when sqlite3 is installed
// #include <sqlite3.h>

namespace codegraph {

// ─────────────────────────────────────────────────────────────────────────────
// Constructor (RAII — acquire the resource)
// ─────────────────────────────────────────────────────────────────────────────
SqliteManager::SqliteManager(const std::string& dbPath)
    : dbPath_(dbPath) {

    // Create the parent directory if it doesn't exist
    std::filesystem::path dir = std::filesystem::path(dbPath).parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }

    // TODO: Open the SQLite database
    // int rc = sqlite3_open(dbPath.c_str(), &db_);
    // if (rc != SQLITE_OK) {
    //     throw StorageError("Failed to open database: " + std::string(sqlite3_errmsg(db_)));
    // }

    std::cout << "[SqliteManager] Database opened: " << dbPath << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// Destructor (RAII — release the resource)
// ─────────────────────────────────────────────────────────────────────────────
SqliteManager::~SqliteManager() {
    // TODO: Close the database
    // if (db_) {
    //     sqlite3_close(db_);
    //     db_ = nullptr;
    // }
    std::cout << "[SqliteManager] Database closed: " << dbPath_ << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// Move constructor
// ─────────────────────────────────────────────────────────────────────────────
SqliteManager::SqliteManager(SqliteManager&& other) noexcept
    : dbPath_(std::move(other.dbPath_))
    // , db_(other.db_)
    , isInitialized_(other.isInitialized_) {
    // other.db_ = nullptr;  // The moved-from object no longer owns the handle
}

SqliteManager& SqliteManager::operator=(SqliteManager&& other) noexcept {
    if (this != &other) {
        // Close our current DB first
        // if (db_) sqlite3_close(db_);

        dbPath_ = std::move(other.dbPath_);
        // db_ = other.db_;
        isInitialized_ = other.isInitialized_;

        // other.db_ = nullptr;
    }
    return *this;
}

// ─────────────────────────────────────────────────────────────────────────────
// Initialize — Set up tables and performance settings
// ─────────────────────────────────────────────────────────────────────────────
void SqliteManager::initialize() {
    if (isInitialized_) return;

    setPragmas();

    // TODO: Create the schema (tables) here, or call schema_migrations
    // The schema includes:
    //   entities(id, name, type, file_path, location, metadata, hash, created_at, updated_at)
    //   relationships(id, from_id, to_id, type, metadata, weight, created_at)
    //   files(path, hash, last_indexed, entity_count)

    isInitialized_ = true;
    std::cout << "[SqliteManager] Database initialized" << std::endl;
}

// ─────────────────────────────────────────────────────────────────────────────
// Set performance pragmas
// ─────────────────────────────────────────────────────────────────────────────
void SqliteManager::setPragmas() {
    // TODO: Execute these SQL pragmas for performance:
    //
    // "PRAGMA journal_mode = WAL"         — Write-Ahead Logging (concurrent reads)
    // "PRAGMA synchronous = NORMAL"       — Faster writes (safe with WAL)
    // "PRAGMA cache_size = -2000"         — 2MB page cache
    // "PRAGMA mmap_size = 268435456"      — 256MB memory-mapped I/O
    // "PRAGMA temp_store = MEMORY"        — Temp tables in RAM
    // "PRAGMA foreign_keys = ON"          — Enforce foreign key constraints
    //
    // Each pragma is like a "setting" for the database engine.
    // WAL mode is the most important — it allows reading while writing.

    std::cout << "[SqliteManager] Pragmas set (placeholder)" << std::endl;
}

bool SqliteManager::isHealthy() const {
    // TODO: Run "SELECT 1" to verify connection is alive
    return isInitialized_;
}

} // namespace codegraph

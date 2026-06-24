///////////////////////////////////////////////////////////////////////////////
/// @file sqlite_manager.h
/// @brief SQLite database connection manager
///
/// MAPS TO ORIGINAL: src/storage/sqlite-manager.ts
///
/// This is the LOWEST level of the storage layer. It manages:
///   1. Opening/closing the SQLite database file
///   2. Setting performance pragmas (WAL mode, cache size, MMAP)
///   3. Providing a connection handle to other storage classes
///
/// PATTERN USED: RAII (Resource Acquisition Is Initialization)
///   - Constructor opens the DB
///   - Destructor closes the DB
///   - This ensures the DB is ALWAYS closed, even if an exception occurs
///
/// BUILD PHASE: 3 (after config/)
///
/// DEPENDENCY: sqlite3 (install via vcpkg: vcpkg install sqlite3)
///
/// WHAT TO LEARN:
///   - RAII: https://learncpp.com/cpp-tutorial/introduction-to-smart-pointers-move-semantics/
///   - SQLite C API: https://www.sqlite.org/cintro.html
///   - WAL mode: https://www.sqlite.org/wal.html
///   - Prepared statements: https://www.sqlite.org/c3ref/prepare.html
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <memory>
// TODO: Uncomment when you install sqlite3 via vcpkg
// #include <sqlite3.h>

namespace codegraph {

class SqliteManager {
public:
    /// Constructor — opens the database (RAII: resource acquired here)
    /// @param dbPath Path to the .db file (created if it doesn't exist)
    explicit SqliteManager(const std::string& dbPath);

    /// Destructor — closes the database (RAII: resource released here)
    ~SqliteManager();

    // ── Delete copy (only ONE manager should own a connection) ──────────
    // WHY: If you copy a SqliteManager, both copies think they own the
    //      same database handle, and both will try to close it. Crash!
    SqliteManager(const SqliteManager&) = delete;
    SqliteManager& operator=(const SqliteManager&) = delete;

    // ── Allow move (transfer ownership) ────────────────────────────────
    SqliteManager(SqliteManager&& other) noexcept;
    SqliteManager& operator=(SqliteManager&& other) noexcept;

    /// Initialize the database (create tables, set pragmas)
    void initialize();

    /// Get the raw SQLite handle (for other storage classes to use)
    /// TODO: Uncomment when sqlite3 is installed
    // sqlite3* getHandle() const { return db_; }

    /// Check if the database is open and healthy
    bool isHealthy() const;

    /// Get the database file path
    const std::string& getPath() const { return dbPath_; }

private:
    std::string dbPath_;
    // sqlite3* db_ = nullptr;  // TODO: Uncomment with sqlite3
    bool isInitialized_ = false;

    /// Set performance pragmas
    /// WAL mode, MMAP, cache size, etc.
    void setPragmas();
};

} // namespace codegraph

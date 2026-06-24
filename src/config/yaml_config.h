///////////////////////////////////////////////////////////////////////////////
/// @file yaml_config.h
/// @brief YAML configuration loader
///
/// MAPS TO ORIGINAL: src/config/yaml-config.ts
///
/// Loads settings from YAML files (config/*.yaml) and merges them
/// with environment variables.
///
/// BUILD PHASE: 2 (after types/)
///
/// DEPENDENCY: yaml-cpp (install via vcpkg: vcpkg install yaml-cpp)
///
/// WHAT TO LEARN:
///   - YAML format: https://learnxinyminutes.com/docs/yaml/
///   - yaml-cpp library: https://github.com/jbeder/yaml-cpp/wiki/Tutorial
///   - Environment variables: std::getenv()
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <optional>

namespace codegraph {

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// AppConfig — All application settings in one struct
//
// Original TS: YamlConfig class loads these from YAML files
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct AppConfig {
    // Database settings
    std::string dbPath           = "data/code-graph.db";
    int         maxConnections   = 5;
    int         cacheSize        = 2000;      // SQLite page cache (pages)
    int64_t     mmapSize         = 268435456; // 256 MB

    // Parser settings
    int         maxFileSize      = 1048576;   // 1 MB
    int         maxDepth         = 10;        // Directory recursion depth

    // Embedding settings
    std::string embeddingProvider = "memory";
    std::string embeddingModel   = "all-MiniLM-L6-v2";
    int         embeddingBatchSize = 8;

    // Server settings
    int         port             = 3000;
    std::string logLevel         = "info";    // "debug", "info", "warn", "error"
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// YamlConfig — The config loader (Facade Pattern)
//
// Usage:
//   auto config = YamlConfig::load("config/default.yaml");
//   std::cout << config.dbPath;  // "data/code-graph.db"
//
// TODO: Implement in yaml_config.cpp
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
class YamlConfig {
public:
    /// Load config from YAML file, merge with defaults and env vars
    static AppConfig load(const std::string& filePath);

    /// Load with environment-based override (e.g., "production.yaml")
    static AppConfig loadForEnvironment(const std::string& environment = "development");

private:
    /// Get environment variable with fallback
    static std::string getEnvOr(const std::string& key, const std::string& defaultValue);
};

} // namespace codegraph

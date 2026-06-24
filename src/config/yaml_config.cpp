///////////////////////////////////////////////////////////////////////////////
/// @file yaml_config.cpp
/// @brief YAML configuration loader — IMPLEMENTATION
///
/// TODO: Fill in the implementation for each method.
///
/// STEP-BY-STEP:
///   1. First, make it work with HARDCODED defaults (no YAML parsing yet)
///   2. Then, add yaml-cpp to parse actual YAML files
///   3. Finally, add environment variable overrides
///
/// DEBUGGING EXERCISE:
///   Set a breakpoint in load(), create a config, and inspect each
///   field in the debugger to verify defaults are correct.
///////////////////////////////////////////////////////////////////////////////

#include "yaml_config.h"
#include <cstdlib>  // for std::getenv
#include <iostream>

namespace codegraph {

// ─────────────────────────────────────────────────────────────────────────────
// Step 1: Start with hardcoded defaults. This ALREADY WORKS.
// Step 2: Replace with yaml-cpp parsing when you're ready.
// ─────────────────────────────────────────────────────────────────────────────
AppConfig YamlConfig::load(const std::string& filePath) {
    AppConfig config;  // All fields have defaults already

    // TODO Step 2: Parse the YAML file
    // #include <yaml-cpp/yaml.h>
    // YAML::Node root = YAML::LoadFile(filePath);
    // if (root["database"]) {
    //     if (root["database"]["path"]) config.dbPath = root["database"]["path"].as<std::string>();
    //     if (root["database"]["max_connections"]) config.maxConnections = root["database"]["max_connections"].as<int>();
    // }

    // TODO Step 3: Override with environment variables
    // config.dbPath = getEnvOr("CODEGRAPH_DB_PATH", config.dbPath);
    // config.logLevel = getEnvOr("CODEGRAPH_LOG_LEVEL", config.logLevel);

    std::cout << "[Config] Loaded config (defaults)" << std::endl;
    std::cout << "[Config]   dbPath: " << config.dbPath << std::endl;
    std::cout << "[Config]   logLevel: " << config.logLevel << std::endl;

    return config;
}

AppConfig YamlConfig::loadForEnvironment(const std::string& environment) {
    std::string filePath = "config/" + environment + ".yaml";
    return load(filePath);
}

std::string YamlConfig::getEnvOr(const std::string& key, const std::string& defaultValue) {
    const char* value = std::getenv(key.c_str());
    return value ? std::string(value) : defaultValue;
}

} // namespace codegraph

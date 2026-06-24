///////////////////////////////////////////////////////////////////////////////
/// @file main.cpp
/// @brief Entry point for the Code Graph RAG system
///
/// This is the LAST file you'll fully implement. Right now it just proves
/// your project compiles. As you build each layer, uncomment the relevant
/// sections and test them here with the debugger.
///
/// BUILD ORDER REMINDER:
///   1. types/       ← You are here! Define data models first
///   2. config/      ← Load YAML configuration
///   3. storage/     ← SQLite database layer
///   4. parsers/     ← Tree-sitter code parsing
///   5. agents/      ← Multi-agent orchestration
///   6. semantic/    ← Embeddings & vector search
///   7. THIS FILE    ← Wire everything together
///
/// DEBUGGING TIPS (Visual Studio):
///   - F9:      Set breakpoint on any line
///   - F5:      Start debugging
///   - F10:     Step over (execute line, don't go into functions)
///   - F11:     Step into (dive into a function call)
///   - Shift+F11: Step out (finish current function, go back)
///   - Hover over variables to see their values
///   - Watch Window: Add variables to monitor their values
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>

// ── Layer 1: Types (Your first implementation) ─────────────────────────────
#include "types/entity_types.h"
#include "types/storage_types.h"
#include "types/parser_types.h"
#include "types/agent_types.h"
#include "types/semantic_types.h"
#include "types/error_types.h"

// ── Layer 2: Config ────────────────────────────────────────────────────────
#include "config/yaml_config.h"

// ── Layer 3: Storage ───────────────────────────────────────────────────────
#include "storage/graph_storage.h"

// ── Uncomment as you build each layer ──────────────────────────────────────
// #include "parsers/tree_sitter_parser.h"
// #include "core/knowledge_bus.h"
// #include "agents/base_agent.h"
// #include "agents/conductor.h"
// #include "semantic/embedding_generator.h"
// #include "semantic/vector_store.h"


int main(int argc, char* argv[]) {
    std::cout << "=== Code Graph RAG (C++ Implementation) ===" << std::endl;
    std::cout << "Version 1.0.0" << std::endl;
    std::cout << std::endl;

    // ========================================================================
    // MILESTONE 1: Test your type system
    // After implementing types/, set a BREAKPOINT here (F9) and inspect
    // the variables in the debugger to verify everything works.
    // ========================================================================
    
    // Create an Entity and inspect it in the debugger
    codegraph::Entity testEntity;
    testEntity.id = "test-entity-001";
    testEntity.name = "main";
    testEntity.type = codegraph::EntityType::Function;
    testEntity.filePath = "src/main.cpp";
    testEntity.location.start = {1, 0, 0};  // line 1, column 0, index 0
    testEntity.location.end = {100, 1, 0};
    testEntity.hash = "abc123";
    testEntity.createdAt = 1716000000000;
    testEntity.updatedAt = 1716000000000;

    std::cout << "[Milestone 1] Entity created: " << testEntity.name 
              << " (type: function)" << std::endl;

    // Create a Relationship and inspect it
    codegraph::Relationship testRel;
    testRel.id = "rel-001";
    testRel.fromId = "test-entity-001";
    testRel.toId = "test-entity-002";
    testRel.type = codegraph::RelationType::Calls;

    std::cout << "[Milestone 1] Relationship created: " 
              << testRel.fromId << " --calls--> " << testRel.toId << std::endl;

    // Create a GraphQuery and inspect it
    codegraph::GraphQuery testQuery;
    testQuery.type = "entity";
    testQuery.filterEntityType = codegraph::EntityType::Function;
    testQuery.limit = 10;
    testQuery.offset = 0;

    std::cout << "[Milestone 1] GraphQuery created: type=" << testQuery.type 
              << ", limit=" << testQuery.limit.value_or(0) << std::endl;

    std::cout << std::endl;
    std::cout << "[SUCCESS] All types compile and work! Move to Milestone 2." << std::endl;

    // ========================================================================
    // MILESTONE 2: Test the config system
    // TODO: Uncomment after implementing config/yaml_config.cpp
    // ========================================================================
    // auto config = codegraph::YamlConfig::load("config/default.yaml");
    // std::cout << "[Milestone 2] Config loaded: db_path=" << config.dbPath << std::endl;

    // ========================================================================
    // MILESTONE 3: Test the storage layer
    // TODO: Uncomment after implementing storage/
    // ========================================================================
    // codegraph::GraphStorage storage("data/test.db");
    // storage.initialize();
    // storage.insertEntity(testEntity);
    // auto retrieved = storage.getEntity("test-entity-001");
    // std::cout << "[Milestone 3] Retrieved: " << retrieved.value().name << std::endl;

    // ========================================================================
    // MILESTONE 4: Test the parser
    // TODO: Uncomment after implementing parsers/
    // ========================================================================
    // codegraph::TreeSitterParser parser;
    // auto result = parser.parseFile("examples/hello.py");
    // std::cout << "[Milestone 4] Parsed " << result.entities.size() << " entities" << std::endl;

    return 0;
}

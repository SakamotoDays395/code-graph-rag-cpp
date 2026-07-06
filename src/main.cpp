#include <iostream>
#include <spdlog/spdlog.h>
#include "storage.hpp"

int main(int argc, char* argv[]) {
    // Basic setup to ensure your dependencies and build system are working
    spdlog::info("Code Graph RAG System - Initialized");
    std::cout << "Ready to build from scratch!\n";

        
    try {
        auto s = code_graph::createSqliteStorage("test_graph.db");
        code_graph::Entity e{};
        e.name = "main";
        e.type = code_graph::EntityType::Function;
        e.filePath = "test.cpp";
        e.location.start = {1, 0, 0};
        e.location.end   = {5, 0, 50};
        e.hash = "deadbeef";
        auto id = s->insertEntity(e);
        auto found = s->getEntity(id);
        if (found && found->name == "main") {
            spdlog::info("Lesson 2 storage smoke test OK");
        } else {
            spdlog::error("Lesson 2 storage smoke test FAILED");
        }
        auto stats = s->getStats();
        spdlog::info("stats: entities={} files={} rels={}",
            stats.totalEntities, stats.totalFiles, stats.totalRelationships);
        s.reset(); // Destroy the storage instance to close the database handle
        std::remove("test_graph.db");  // cleanup
    } catch (const std::exception& ex) {
        spdlog::critical("Exception caught in main: {}", ex.what());
    }
    
    // TODO: Initialize your graph storage
    // TODO: Initialize your tree-sitter parsers
    // TODO: Run the main RAG loop
    
    return 0;
}

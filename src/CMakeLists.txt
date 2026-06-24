###############################################################################
# src/ CMakeLists.txt — Defines all library targets (one per architectural layer)
###############################################################################

# ── Layer 1: Types (Data Models — no dependencies) ─────────────────────────
add_library(types_lib STATIC
    types/types.cpp           # Stub compilation unit (includes all headers)
    types/entity_types.h      # EntityType, RelationType enums
    types/storage_types.h     # Entity, Relationship, GraphQuery structs
    types/parser_types.h      # ParsedEntity, ParseResult structs
    types/agent_types.h       # AgentTask, AgentStatus structs
    types/semantic_types.h    # EmbeddingConfig, SimilarityResult structs
    types/error_types.h       # Custom error classes
  )

# ── Layer 2: Config (depends on: types) ────────────────────────────────────
add_library(config_lib STATIC
    config/yaml_config.h
    config/yaml_config.cpp
  )
target_link_libraries(config_lib PRIVATE types_lib)
target_include_directories(types_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_link_libraries(config_lib PRIVATE yaml-cpp::yaml-cpp)  # TODO: Uncomment with vcpkg

# ── Layer 3: Storage (depends on: types) ───────────────────────────────────
add_library(storage_lib STATIC
    storage/sqlite_manager.h
    storage/sqlite_manager.cpp
    storage/schema_migrations.h
    storage/schema_migrations.cpp
    storage/graph_storage.h
    storage/graph_storage.cpp
    storage/batch_operations.h
    storage/batch_operations.cpp
    storage/cache_manager.h
    storage/cache_manager.cpp
  )
target_link_libraries(storage_lib PRIVATE types_lib)
# target_link_libraries(storage_lib PRIVATE unofficial::sqlite3::sqlite3)  # TODO: Uncomment

# ── Layer 4: Parsers (depends on: types) ───────────────────────────────────
# add_library(parsers_lib STATIC
#     parsers/tree_sitter_parser.h
#     parsers/tree_sitter_parser.cpp
#     parsers/python_analyzer.h
#     parsers/python_analyzer.cpp
#     parsers/cpp_analyzer.h
#     parsers/cpp_analyzer.cpp
# )
# target_link_libraries(parsers_lib PRIVATE types_lib)

# ── Layer 5: Core Infrastructure (depends on: types) ──────────────────────
# add_library(core_lib STATIC
#     core/knowledge_bus.h
#     core/knowledge_bus.cpp
#     core/resource_manager.h
#     core/resource_manager.cpp
# )
# target_link_libraries(core_lib PRIVATE types_lib)

# ── Layer 6: Agents (depends on: types, storage, parsers, core) ───────────
add_library(agents_lib STATIC
    agents/base_agent.h
    agents/base_agent.cpp
    #agents/parser_agent.h
    #agents/parser_agent.cpp
    #agents/indexer_agent.h
    #agents/indexer_agent.cpp
    #agents/conductor.h
    #agents/conductor.cpp
)
target_link_libraries(agents_lib PRIVATE types_lib storage_lib) #parsers_lib core_lib)
target_include_directories(agents_lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
# ── Layer 7: Semantic (depends on: types, storage) ────────────────────────
# add_library(semantic_lib STATIC
#     semantic/embedding_generator.h
#     semantic/embedding_generator.cpp
#     semantic/vector_store.h
#     semantic/vector_store.cpp
# )
# target_link_libraries(semantic_lib PRIVATE types_lib storage_lib)

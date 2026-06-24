# Code Graph RAG — C++ Implementation

A C++ replication of the [code-graph-rag-mcp](https://github.com/nicobailey/code-graph-rag-mcp) TypeScript project, built as a learning exercise.

## 🎯 What Is This?

An MCP server that:
1. **Parses** source code into an Abstract Syntax Tree (AST) using Tree-Sitter
2. **Stores** code entities (functions, classes) and relationships (calls, imports) in a SQLite graph database
3. **Generates** vector embeddings for semantic code search
4. **Serves** 26 tools to AI assistants (Claude, Gemini) via the MCP protocol

## 📁 Project Structure

```
code-graph-rag-cpp/
├── CMakeLists.txt              ← Open this in Visual Studio
├── config/default.yaml         ← Application settings
├── examples/hello.py           ← Test files for parser
├── src/
│   ├── main.cpp                ← Entry point (milestone tests)
│   ├── types/                  ← ⭐ START HERE: Data models
│   │   ├── entity_types.h      ← Enums (EntityType, RelationType)
│   │   ├── storage_types.h     ← Core structs (Entity, Relationship)
│   │   ├── parser_types.h      ← Parser output types
│   │   ├── agent_types.h       ← Agent system types
│   │   ├── semantic_types.h    ← Embedding/vector types
│   │   └── error_types.h       ← Custom exceptions
│   ├── config/                 ← YAML config loading
│   ├── storage/                ← SQLite graph database
│   │   ├── sqlite_manager.*    ← Connection + pragmas
│   │   ├── schema_migrations.* ← Table creation
│   │   ├── graph_storage.*     ← CRUD operations (Facade)
│   │   ├── batch_operations.*  ← Bulk inserts
│   │   └── cache_manager.h     ← LRU cache (complete!)
│   ├── parsers/                ← [TODO] Tree-sitter parsing
│   ├── agents/                 ← [TODO] Multi-agent system
│   ├── semantic/               ← [TODO] Embedding + vector search
│   ├── core/                   ← [TODO] Knowledge bus
│   └── tools/                  ← [TODO] MCP tool implementations
└── tests/                      ← [TODO] Unit tests
```

## 🚀 How to Build (Visual Studio Community)

### Prerequisites
1. [Visual Studio Community 2022](https://visualstudio.microsoft.com/) with "Desktop development with C++" workload
2. [vcpkg](https://vcpkg.io/) for dependency management

### Install Dependencies
```powershell
# Install vcpkg (one-time setup)
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install project dependencies
.\vcpkg install sqlite3 yaml-cpp nlohmann-json gtest
```

### Open in Visual Studio
1. Open Visual Studio
2. **File → Open → CMake...**
3. Navigate to `CMakeLists.txt` in this folder
4. Visual Studio auto-generates the solution
5. **Build → Build All** (Ctrl+Shift+B)
6. **Debug → Start Debugging** (F5)

## 🏗️ Build Order (Follow This Exactly)

| Phase | Milestone | What to Implement |
|-------|-----------|-------------------|
| 1 | ✅ Types compile | `src/types/` — Already done! |
| 2 | Config loads | `src/config/` — Load YAML settings |
| 3 | DB works | `src/storage/` — SQLite CRUD |
| 4 | Parser works | `src/parsers/` — Tree-sitter AST |
| 5 | Agents run | `src/agents/` — Task delegation |
| 6 | Search works | `src/semantic/` — Embeddings |
| 7 | Server runs | `main.cpp` — MCP JSON-RPC |

## 🐛 Debugging Tips

- **F9**: Set breakpoint
- **F5**: Start debugging
- **F10**: Step over (execute line)
- **F11**: Step into (dive into function)
- **Shift+F11**: Step out
- **Watch Window**: Add variables to monitor
- **DB Browser for SQLite**: Open .db files to inspect tables

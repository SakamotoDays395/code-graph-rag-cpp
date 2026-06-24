///////////////////////////////////////////////////////////////////////////////
/// @file error_types.h
/// @brief Custom error/exception classes
///
/// MAPS TO ORIGINAL: src/types/errors.ts
///
/// WHAT TO LEARN:
///   - C++ exceptions: https://learncpp.com/cpp-tutorial/exceptions/
///   - Custom exception classes
///   - std::runtime_error inheritance
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include <stdexcept>
#include <string>
namespace codegraph {
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // Base error for all Code Graph RAG errors
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    class CodeGraphError : public std::runtime_error {
    public:
        explicit CodeGraphError(const std::string& message)
            : std::runtime_error("[CodeGraphRAG] " + message) {}
    };
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // Storage layer errors
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    class StorageError : public CodeGraphError {
    public:
        explicit StorageError(const std::string& message)
            : CodeGraphError("[Storage] " + message) {
        }
    };
    class EntityNotFoundError : public StorageError {
    public:
        explicit EntityNotFoundError(const std::string& entityId)
            : StorageError("Entity not found: " + entityId) {
        }
    };
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // Parser layer errors
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    class ParseError : public CodeGraphError {
    public:
        explicit ParseError(const std::string& message)
            : CodeGraphError("[Parser] " + message) {
        }
    };
    class UnsupportedLanguageError : public ParseError {
    public:
        explicit UnsupportedLanguageError(const std::string& language)
            : ParseError("Unsupported language: " + language) {
        }
    };
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    // Agent layer errors
    // ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    class AgentError : public CodeGraphError {
    public:
        explicit AgentError(const std::string& message)
            : CodeGraphError("[Agent] " + message) {
        }
    };
    class AgentBusyError : public AgentError {
    public:
        explicit AgentBusyError(const std::string& agentId)
            : AgentError("Agent busy, queue full: " + agentId) {
        }
    };
    // TODO: Add more error types as you build each layer:
    // class ConfigError : public CodeGraphError { ... };
    // class EmbeddingError : public CodeGraphError { ... };
    // class QueryError : public CodeGraphError { ... };
} // namespace codegraph

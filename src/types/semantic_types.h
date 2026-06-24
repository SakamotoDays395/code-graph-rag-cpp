///////////////////////////////////////////////////////////////////////////////
/// @file semantic_types.h
/// @brief Types for embedding generation and vector search
///
/// MAPS TO ORIGINAL: src/types/semantic.ts
///
/// These define how code is converted to vectors (numbers) for AI search.
/// You'll implement this in Phase 5 (Semantic Layer).
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <cstdint>

namespace codegraph {

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// Vector dimensions — all embeddings must be this size
// The MiniLM model produces 384-dimensional vectors
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
constexpr int VECTOR_DIMENSIONS = 384;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// EmbeddingConfig — Settings for the embedding generator
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct EmbeddingConfig {
    std::string modelName = "all-MiniLM-L6-v2";
    bool        quantized = true;   // Use smaller model for speed
    std::string localPath = "./models";
    int         batchSize = 8;
    std::string provider  = "memory"; // "memory", "onnx", "openai", "ollama"

    // TODO: Add provider-specific configs when you build them:
    // struct OllamaConfig { std::string baseUrl; std::string model; };
    // struct OpenAIConfig { std::string apiKey; std::string model; };
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// SimilarityResult — A single search result from vector similarity search
//
// When you search "find code similar to authentication",
// each matching entity comes back as a SimilarityResult with a score.
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct SimilarityResult {
    std::string entityId;
    double      similarity = 0.0; // 0.0 = unrelated, 1.0 = identical
    std::string content;          // The code/text that matched

    // TODO: Add when you build hybrid search:
    // double graphScore = 0.0;
    // double combinedScore = 0.0;
};

} // namespace codegraph

#pragma once

#include "storage.hpp"
#include "parser.hpp"
#include "logger.hpp"

#include <nlohmann/json.hpp>

#include <functional>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace code_graph {

struct ToolContext {
    IGraphStorage* storage = nullptr;
    Parser* parser = nullptr;
    std::string workspaceRoot;
};

// a tool handler is a pure function
using ToolHandler = std::function<nlohmann::json(const nlohmann::json&)>;

struct ToolDefinition {
    std::string name;
    std::string description;
    nlohmann::json inputSchema;
    ToolHandler handler;
};

class ToolRegistry {
    public:
    void registerTool(ToolDefinition def);
    const ToolDefinition* find(const std::string& name) const;
    std::vector<const ToolDefinition*> all() const;
    private:
    std::unordered_map<std::string, ToolDefinition> tools_;
};

void registerAllTools(ToolRegistry& registry, ToolContext& ctx);

}
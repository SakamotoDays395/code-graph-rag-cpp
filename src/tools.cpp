#include "tools.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace code_graph {
void ToolRegistry::registerTool(ToolDefinition def) {
    tools_[def.name] = std::move(def);
}
const ToolDefinition* ToolRegistry::find(const std::string& name) const {
    auto it = tools_.find(name);
    return it != tools_.end() ? &it->second : nullptr;
}
std::vector<const ToolDefinition*> ToolRegistry::all() const {
    std::vector<const ToolDefinition*> out;
    out.reserve(tools_.size());
    for (const auto& [_, tool] : tools_) {
        out.push_back(&tool);
    }
    std::sort(out.begin(), out.end(), [](const ToolDefinition* a, const ToolDefinition* b) {return a->name < b->name; });
    return out;
}
namespace {
using Json = nlohmann::json;

std::string resolvePath(const ToolContext& ctx, const std::string& path) {
    if (path.empty()) return path;
    if (path[0] == '/' || (path.size() >= 2 && path[1] == ':')) return path;
    if (ctx.workspaceRoot.empty()) return path;
    std::string base = ctx.workspaceRoot;
    if (base.back() != '/' && base.back() != '\\') base += "/";
    return base + path;
}
Json errorPayload(const std::string& code, const std::string& message) {
    return Json{
            {"success", false},
            {"error", Json{{"code", code}, {"message", message}}}
    };
}

Json successPayload(Json data) {
    Json p{
                {"success", true}
    };
    p["data"] = std::move(data);
    return p;
}

Json entityToJson(const Entity& e) {
    Json j{
                {"id", e.id},
                {"name", e.name},
                {"type", std::string(entityTypeToString(e.type))},
                {"filePath", e.filePath},
                {"location", Json{
                    {"start",Json{{"line", e.location.start.line},
                                                {"column", e.location.start.column},
                                                    {"index", e.location.start.index}}},
                       {"end", Json{{"line", e.location.end.line},
                                            {"column", e.location.end.column},
                                                {"index", e.location.end.index}}}
                }},
            {"hash", e.hash},
            {"createdAt", e.createdAt},
            {"updatedAt", e.updatedAt}
    };
    if (e.metadata.signature)     j["signature"]     = *e.metadata.signature;
    if (e.metadata.returnType)     j["returnType"]    = *e.metadata.returnType;
    if (e.metadata.modifiers)      j["modifiers"]     = *e.metadata.modifiers;
    if (e.metadata.templateParams) j["templateParams"] = *e.metadata.templateParams;
    if (e.metadata.baseClasses)    j["baseClasses"]    = *e.metadata.baseClasses;
    return j;
}
Json relationshipToJson(const Relationship& r) {
    Json j{
        {"id",       r.id},
        {"fromId",   r.fromId},
        {"toId",     r.toId},
        {"type",     std::string(relationTypeToString(r.type))},
        {"createdAt", r.createdAt}
    };
    if (r.metadata) {
        Json meta = Json::object();
        if (r.metadata->line)    meta["line"]    = *r.metadata->line;
        if (r.metadata->context) meta["context"] = *r.metadata->context;
        if (!meta.empty()) j["metadata"] = std::move(meta);
    }
    return j;
}

Json handleIndexFile(const ToolContext& ctx, const Json& args) {
    auto path = args.value("path", std::string{});
    if (path.empty()) return errorPayload("invalid_args", "path is required");
    path = resolvePath(ctx, path);
    if (!ctx.parser) return errorPayload("config_error", "parser not initialized");
    try {
        auto t0 = std::chrono::steady_clock::now();
        ParseResult r = ctx.parser->parseFile(path);
        auto t1 = std::chrono::steady_clock::now();
        int totalMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

        Logger::global().debug("tools","index_file: path = {} ok = {}, entities = {}, rels = {}", path, r.ok ? "true" : "false", r.entitiesEmitted, r.relationshipsEmitted);
        if (!r.ok) {
            return errorPayload("parse_failed", r.errorMessage);
        }
        if (ctx.storage) {
            FileInfo info;
            info.path = path;
            info.hash = std::to_string(r.entitiesEmitted);
            info.entityCount = r.entitiesEmitted;
            ctx.storage->upsertFileInfo(info);
        }
        return successPayload(Json{
            {"filePath", path},
            {"entitiesEmitted", r.entitiesEmitted},
            {"relationshipsEmitted", r.relationshipsEmitted},
            {"durationMs", std::max(totalMs, r.durationMs)}
        });
    } catch (const std::exception& ex) {
        Logger::global().error("tools","error: {}", ex.what());
        return errorPayload("exception", ex.what());
    }
}

ToolDefinition makeIndexFileTool(ToolContext& ctx) {
    ToolDefinition def;
    def.name = "index_file";
    def.description = "Parse a single C++ source file and insert discovered entities/relationships into the graph database.";
    def.inputSchema = Json{
            {"type", "object"},
            {"properties", Json{
                {"path", Json{{"type", "string"},
                              {"description", "Path to the .cpp/.hpp/.h file. Relative paths resolve against the workspace root."}}}
            }},
            {"required", Json::array({"path"})}
    };
    def.handler = [&ctx](const Json& args) { return handleIndexFile(ctx, args); };
    return def;
}

// TOOL 2
EntityType stringToEntityTypeSafe(const std::string& s) {
    if (s == "function")  return EntityType::Function;
    if (s == "class")     return EntityType::Class;
    if (s == "method")    return EntityType::Method;
    if (s == "struct")    return EntityType::Struct;
    if (s == "variable")  return EntityType::Variable;
    if (s == "field")     return EntityType::Field;
    if (s == "namespace") return EntityType::Namespace;
    if (s == "enum")      return EntityType::Enum;
    if (s == "typedef")   return EntityType::Typedef;
    if (s == "template")  return EntityType::Template;
    throw std::invalid_argument("Unknown entity type: " + s);
}

Json handleGetGraph(const ToolContext& ctx, const Json& args) {
    if (!ctx.storage) {
        return errorPayload("config_error", "storage not initialized");
    }
    EntityQuery query;
    if (args.contains("type"))     query.type     = stringToEntityTypeSafe(args["type"].get<std::string>());
    if (args.contains("filePath")) query.filePath = args["filePath"].get<std::string>();
    if (args.contains("name"))     query.name     = args["name"].get<std::string>();
    if (args.contains("limit"))    query.limit    = args["limit"].get<int>();
    if (args.contains("offset"))   query.offset   = args["offset"].get<int>();
    auto entities = ctx.storage->findEntities(query);
    Json arr = Json::array();
    for (const auto& e : entities) arr.push_back(entityToJson(e));

    auto stats = ctx.storage->getStats();
    return successPayload(Json{
        {"entities", std::move(arr)},
        {"stats", Json{
            {"returned", static_cast<int>(entities.size())},
            {"totalEntitiesInDb", stats.totalEntities},
            {"totalRelationshipsInDb", stats.totalRelationships},
            {"totalFilesInDb", stats.totalFiles}
        }}
    });
}

ToolDefinition makeGetGraphTool(ToolContext& ctx) {
    ToolDefinition def;
    def.name = "get_graph";
    def.description = "Query the code graph. Returns entities matching the given filters. Use to explore functions, classes, namespaces, and their relationships.";
    def.inputSchema = Json{
        {"type", "object"},
        {"properties", Json{
            {"type",     Json{{"type", "string"}, {"description", "Filter by entity type: function, class, method, struct, variable, field, namespace, enum, typedef, template"}}},
            {"filePath", Json{{"type", "string"}, {"description", "Filter by source file path (exact match)"}}},
            {"name",     Json{{"type", "string"}, {"description", "Filter by entity name (exact match)"}}},
            {"limit",    Json{{"type", "integer"}, {"description", "Max results"}, {"default", 100}}},
            {"offset",   Json{{"type", "integer"}, {"description", "Pagination offset"}, {"default", 0}}}
        }}
    };
    def.handler = [&ctx](const Json& args) { return handleGetGraph(ctx, args); };
    return def;
}


// TOOL 3

Json handleGetEntity(const ToolContext& ctx, const Json& args) {
    auto id = args.value("id", std::string{});
    if (id.empty()) {
        return errorPayload("invalid_args", "id is required");
    }
    if (!ctx.storage) {
        return errorPayload("config_error", "storage not initialized");
    }
    auto entity = ctx.storage->getEntity(id);
    if (!entity) {
        return errorPayload("not_found", "entity not found: " + id);
    }
    auto rels = ctx.storage->getRelationshipsForEntity(id);
    Json relArr = Json::array();
    for (const auto& r : rels) relArr.push_back(relationshipToJson(r));

    return successPayload(Json{
        {"entity", entityToJson(*entity)},
        {"relationships", std::move(relArr)}
    });
}

ToolDefinition makeGetEntityTool(ToolContext& ctx) {
    ToolDefinition def;
    def.name = "get_entity";
    def.description = "Fetch a single entity by its id, along with all relationships (incoming and outgoing) connected to it.";
    def.inputSchema = Json{
            {"type", "object"},
            {"properties", Json{
                {"id", Json{{"type", "string"}, {"description", "Entity id (the synthetic `filePath|type|name|start-end` key)"}}}
            }},
            {"required", Json::array({"id"})}
    };
    def.handler = [&ctx](const Json& args) { return handleGetEntity(ctx, args); };
    return def;
}

}  // namespace

void registerAllTools(ToolRegistry& registry, ToolContext& ctx) {
    registry.registerTool(makeIndexFileTool(ctx));
    registry.registerTool(makeGetGraphTool(ctx));
    registry.registerTool(makeGetEntityTool(ctx));
    Logger::global().info("tools", "registered 3 tools: index_file, get_graph, get_entity");

}
}//code_graph
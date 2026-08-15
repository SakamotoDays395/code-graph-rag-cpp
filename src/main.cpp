// ════════════════════════════════════════════════════════════════════
//  main.cpp  ·  MCP JSON-RPC stdio server (Lesson 6)
//
//  This is the composition root — the ONLY place that knows about
//  concrete types (SqliteGraphStorage, Parser). Everything else
//  depends on abstractions (IGraphStorage, ToolContext).
//
//  CRITICAL: stdout carries ONLY JSON-RPC frames. All diagnostics
//  go to stderr via Logger. Mixing logs into stdout corrupts the
//  MCP stream and breaks every IDE client.
// ════════════════════════════════════════════════════════════════════

#include "tools.hpp"
#include "logger.hpp"
#include "storage.hpp"
#include "parser.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace {

using Json = nlohmann::json;

// ── String helpers for header parsing ───────────────────────────────

std::string trim(std::string value) {
    while (!value.empty() && (value.back() == '\r' || value.back() == ' ' || value.back() == '\t'))
        value.pop_back();
    std::size_t start = 0;
    while (start < value.size() && (value[start] == ' ' || value[start] == '\t'))
        ++start;
    return value.substr(start);
}

std::string lowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

// ── Frame I/O (LSP-style Content-Length framing) ────────────────────
//
// Concept: framed binary protocol. The body can contain \r\n inside
// JSON strings, so we must read by byte count, not by getline.

bool readJsonRpcFrame(std::istream& in, std::string& body) {
    std::string line;
    std::size_t contentLength = 0;
    bool foundContentLength = false;

    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            break;

        const std::size_t colon = line.find(':');
        if (colon == std::string::npos)
            continue;

        const std::string headerName = lowerCopy(trim(line.substr(0, colon)));
        const std::string headerValue = trim(line.substr(colon + 1));
        if (headerName == "content-length") {
            try {
                contentLength = static_cast<std::size_t>(std::stoul(headerValue));
                foundContentLength = true;
            } catch (...) {
                return false;
            }
        }
    }

    if (!foundContentLength)
        return false;

    body.assign(contentLength, '\0');
    in.read(body.data(), static_cast<std::streamsize>(contentLength));
    return static_cast<std::size_t>(in.gcount()) == contentLength;
}

void writeJsonRpcFrame(const Json& payload) {
    const std::string body = payload.dump();
    std::cout << "Content-Length: " << body.size() << "\r\n\r\n";
    std::cout << body;
    std::cout.flush();
}

// ── JSON-RPC response builders ──────────────────────────────────────

Json makeResultResponse(const Json& id, const Json& result) {
    return Json{{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
}

Json makeErrorResponse(const Json& id, int code, const std::string& message,
                       const Json& data = Json()) {
    Json error = {{"code", code}, {"message", message}};
    if (!data.is_null())
        error["data"] = data;
    return Json{{"jsonrpc", "2.0"}, {"id", id}, {"error", error}};
}

// ── MCP tool result wrapper ─────────────────────────────────────────
//
// Converts a tool handler's JSON payload into the MCP tools/call
// result format. isError is derived from the payload's "success" flag.

Json toMcpToolResult(const Json& payload) {
    return Json{
        {"isError", !payload.value("success", false)},
        {"content", Json::array({Json{{"type", "text"}, {"text", payload.dump(2)}}})},
        {"structuredContent", payload}
    };
}

// ── tools/list response builder ─────────────────────────────────────

Json listToolsResult(const code_graph::ToolRegistry& registry) {
    Json toolsList = Json::array();
    for (const auto* tool : registry.all()) {
        toolsList.push_back(Json{
            {"name", tool->name},
            {"description", tool->description},
            {"inputSchema", tool->inputSchema}
        });
    }
    return Json{{"tools", toolsList}};
}

}  // namespace

// ════════════════════════════════════════════════════════════════════
//  main — the composition root + blocking JSON-RPC loop
// ════════════════════════════════════════════════════════════════════

int main() {
#ifdef _WIN32
    // CRITICAL: binary mode prevents \n → \r\n translation that
    // corrupts Content-Length byte counts in JSON-RPC frames.
    _setmode(_fileno(stdin),  _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // ── Composition root: wire concrete types together ──────────────

    std::string dbPath = "code_graph.db";
    if (const char* env = std::getenv("CODE_GRAPH_DB"); env && *env)
        dbPath = env;

    auto storage = code_graph::createSqliteStorage(dbPath);
    code_graph::Parser parser(*storage);

    std::string workspaceRoot;
    if (const char* env = std::getenv("MCP_WORKSPACE_ROOT"); env && *env)
        workspaceRoot = env;

    code_graph::ToolContext ctx{storage.get(), &parser, workspaceRoot};
    code_graph::ToolRegistry tools;
    code_graph::registerAllTools(tools, ctx);

    spdlog::info("[mcp] code-graph-rag server started (db={})", dbPath);

    // ── Blocking JSON-RPC loop ──────────────────────────────────────

    std::string frame;
    while (readJsonRpcFrame(std::cin, frame)) {
        Json request;
        try {
            request = Json::parse(frame);
        } catch (const std::exception& e) {
            spdlog::warn("[mcp] parse error: {}", e.what());
            writeJsonRpcFrame(makeErrorResponse(nullptr, -32700, "Parse error"));
            continue;
        }

        const bool isRequest = request.contains("id");
        const Json id = request.value("id", Json(nullptr));
        const std::string method = request.value("method", std::string());
        const Json params = request.value("params", Json::object());

        try {
            if (method == "initialize") {
                Json result = Json::object();
                result["protocolVersion"] = "2025-03-26";
                result["capabilities"] = Json::object();
                result["capabilities"]["tools"] = Json::object();
                result["capabilities"]["tools"]["listChanged"] = false;
                result["serverInfo"] = Json::object();
                result["serverInfo"]["name"] = "code-graph-rag-cpp";
                result["serverInfo"]["version"] = "0.1.0";
                if (isRequest)
                    writeJsonRpcFrame(makeResultResponse(id, result));
                continue;
            }

            if (method == "notifications/initialized") {
                continue;
            }

            if (method == "ping") {
                if (isRequest)
                    writeJsonRpcFrame(makeResultResponse(id, Json::object()));
                continue;
            }

            if (method == "tools/list") {
                if (isRequest)
                    writeJsonRpcFrame(makeResultResponse(id, listToolsResult(tools)));
                continue;
            }

            if (method == "tools/call") {
                if (!isRequest)
                    continue;

                const std::string name = params.value("name", std::string());
                const Json arguments = params.value("arguments", Json::object());

                if (name.empty()) {
                    writeJsonRpcFrame(makeErrorResponse(id, -32602, "Invalid params",
                        Json{{"reason", "Tool name is required"}}));
                    continue;
                }

                const auto* def = tools.find(name);
                if (!def) {
                    writeJsonRpcFrame(makeErrorResponse(id, -32601, "Unknown tool",
                        Json{{"name", name}}));
                    continue;
                }

                const Json payload = def->handler(arguments);
                writeJsonRpcFrame(makeResultResponse(id, toMcpToolResult(payload)));
                continue;
            }

            if (isRequest) {
                writeJsonRpcFrame(makeErrorResponse(id, -32601, "Method not found",
                    Json{{"method", method}}));
            }
        } catch (const std::exception& e) {
            spdlog::error("[mcp] request failed: {}", e.what());
            if (isRequest) {
                writeJsonRpcFrame(makeErrorResponse(id, -32603, "Internal error",
                    Json{{"what", e.what()}}));
            }
        }
    }

    spdlog::info("[mcp] code-graph-rag server stopped");
    return 0;
}

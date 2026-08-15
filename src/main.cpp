#include "logger.hpp"

#include "parser.hpp"
#include "storage.hpp"
#include "tools.hpp"
#include <fstream>
#include <cstdio>

int main(int argc, char* argv[]) {
    try {
        auto storage = code_graph::createSqliteStorage("test_tools.db");
        code_graph::Parser parser(*storage);
        code_graph::ToolContext ctx{storage.get(), &parser, ""};
        code_graph::ToolRegistry tools;
        code_graph::registerAllTools(tools, ctx);

        // Test 1: list all tools
        code_graph::log_info("test", "--- Registered tools ---");
        for (const auto* t : tools.all()) {
            code_graph::log_info("test", "  name={}", t->name);
        }

        // Test 2: index a file
        std::ofstream("test_tools_target.cpp") << R"CPP(
namespace physics {
    class Body {
    public:
        float mass = 1.0f;
        float velocity(float dt) { return mass * dt; }
    };
}
)CPP";

        auto indexResult = tools.find("index_file")->handler(
            nlohmann::json{{"path", "test_tools_target.cpp"}});
        code_graph::log_info("test", "index_file result: {}", indexResult.dump(2));

        // Test 3: get_graph (all entities)
        auto graphResult = tools.find("get_graph")->handler(
            nlohmann::json{{"limit", 100}});
        code_graph::log_info("test", "get_graph returned {} entities",
            graphResult["data"]["entities"].size());

        // Test 4: get_entity (first entity's id)
        if (!graphResult["data"]["entities"].empty()) {
            std::string firstId = graphResult["data"]["entities"][0]["id"];
            auto entityResult = tools.find("get_entity")->handler(
                nlohmann::json{{"id", firstId}});
            code_graph::log_info("test", "get_entity result: {}", entityResult.dump(2));
        }

        // Test 5: error path — unknown tool
        auto* unknown = tools.find("nonexistent_tool");
        code_graph::log_info("test", "unknown tool lookup returned null: {}", (unknown == nullptr));

        std::remove("test_tools_target.cpp");
        std::remove("test_tools.db");
        std::remove("test_tools.db-wal");
        std::remove("test_tools.db-shm");
        std::remove("code_graph.log");
    } catch (const std::exception& ex) {
        code_graph::log_error("test", "EXCEPTION: {}", ex.what());
        return 1;
    }

    return 0;
}

#include <iostream>
#include <spdlog/spdlog.h>

#include "parser.hpp"
#include "storage.hpp"
#include <fstream>
#include <cstdio>

int main(int argc, char* argv[]) {
    try {
        spdlog::info("Code Graph RAG System - Stress Test");

        // A complex C++ file with nested namespaces, inheritance, 
        // multiple call chains, structs, and variables
        std::ofstream("test_complex.cpp") << R"CPP(
#include <string>
#include <vector>
#include <memory>

namespace engine {
namespace math {

    struct Vec3 {
        float x, y, z;
        float length() const { return sqrt(x*x + y*y + z*z); }
        Vec3 normalize() const {
            float len = length();
            return {x/len, y/len, z/len};
        }
    };

    Vec3 cross(const Vec3& a, const Vec3& b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    float dot(const Vec3& a, const Vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

} // namespace math

namespace core {

    class Component {
    public:
        virtual ~Component() = default;
        virtual void update(float deltaTime) = 0;
        virtual std::string typeName() const = 0;
    };

    class Entity {
    public:
        Entity(const std::string& name) : m_name(name) {}

        void addComponent(std::unique_ptr<Component> comp) {
            m_components.push_back(std::move(comp));
        }

        void tick(float dt) {
            for (auto& c : m_components) {
                c->update(dt);
            }
        }

        const std::string& name() const { return m_name; }

    private:
        std::string m_name;
        std::vector<std::unique_ptr<Component>> m_components;
    };

    class Transform : public Component {
    public:
        math::Vec3 position;
        math::Vec3 rotation;
        math::Vec3 scale;

        void update(float deltaTime) override {
            position.normalize();
        }

        std::string typeName() const override { return "Transform"; }
    };

    class RigidBody : public Component {
    public:
        math::Vec3 velocity;
        math::Vec3 acceleration;
        float mass = 1.0f;

        void update(float deltaTime) override {
            float speed = velocity.length();
            velocity = math::cross(velocity, acceleration);
            applyGravity(deltaTime);
        }

        std::string typeName() const override { return "RigidBody"; }

    private:
        void applyGravity(float dt) {
            acceleration.y -= 9.81f * dt;
        }
    };

} // namespace core

namespace scene {

    class Scene {
    public:
        void addEntity(std::unique_ptr<core::Entity> entity) {
            m_entities.push_back(std::move(entity));
        }

        void simulate(float dt) {
            for (auto& e : m_entities) {
                e->tick(dt);
            }
        }

        int entityCount() const { return static_cast<int>(m_entities.size()); }

    private:
        std::vector<std::unique_ptr<core::Entity>> m_entities;
    };

    Scene createDefaultScene() {
        Scene s;
        auto player = std::make_unique<core::Entity>("Player");
        auto transform = std::make_unique<core::Transform>();
        transform->position = {0.0f, 1.0f, 0.0f};
        player->addComponent(std::move(transform));

        auto rb = std::make_unique<core::RigidBody>();
        rb->mass = 75.0f;
        player->addComponent(std::move(rb));

        s.addEntity(std::move(player));
        return s;
    }

} // namespace scene

} // namespace engine

int main() {
    auto scene = engine::scene::createDefaultScene();
    scene.simulate(0.016f);
    return scene.entityCount();
}
        )CPP";

        spdlog::info("Complex test file written, creating storage...");
        auto storage = code_graph::createSqliteStorage("test_complex_graph.db");
        code_graph::Parser parser(*storage);

        spdlog::info("Parsing complex file...");
        auto r = parser.parseFile("test_complex.cpp");
        spdlog::info("parse: ok={} entities={} rels={} ms={}",
                     r.ok, r.entitiesEmitted, r.relationshipsEmitted, r.durationMs);

        auto stats = storage->getStats();
        spdlog::info("stats: entities={} files={} rels={}",
                     stats.totalEntities, stats.totalFiles, stats.totalRelationships);

        // List ALL entities
        spdlog::info("--- ENTITIES ---");
        code_graph::EntityQuery query;
        query.limit = 200;
        auto es = storage->findEntities(query);
        for (const auto& e : es) {
            spdlog::info("  [{}] name={:<25} file={} line={}",
                         std::string(code_graph::entityTypeToString(e.type)),
                         e.name, e.filePath, e.location.start.line);
        }

        std::remove("test_complex.cpp");
        std::remove("test_complex_graph.db");
        std::remove("test_complex_graph.db-wal");
        std::remove("test_complex_graph.db-shm");
    } catch (const std::exception& ex) {
        spdlog::error("EXCEPTION: {}", ex.what());
        return 1;
    }

    return 0;
}

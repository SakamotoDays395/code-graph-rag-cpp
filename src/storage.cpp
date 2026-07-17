#include "storage.hpp"
//SQlit library headers mentioned in .cpp rather than headers to .exe unaware of underthehood database.(CMakeLists.txt)
#include <SQLiteCpp/SQLiteCpp.h>
#include <sqlite3.h>

#include <chrono>
#include <stdexcept>
#include <sstream>
#include <nlohmann/json.hpp>

namespace code_graph {
namespace{
    //anonymous namespace to make helper funcitons confined to this .cpp file.
    
    std::int64_t nowMillis() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    constexpr const char* kSchemaV1 = R"SQL(
        CREATE TABLE IF NOT EXISTS entities (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            type TEXT NOT NULL,
            file_path TEXT NOT NULL,
            start_line INTEGER NOT NULL,
            start_column INTEGER NOT NULL,
            start_index INTEGER NOT NULL,
            end_line INTEGER NOT NULL,
            end_column INTEGER NOT NULL,
            end_index INTEGER NOT NULL,
            signature TEXT,
            return_type TEXT,
            modifiers TEXT,
            template_params TEXT,
            base_classes TEXT,
            hash TEXT,
            created_at INTEGER NOT NULL,
            updated_at INTEGER NOT NULL
        );
        
        CREATE TABLE IF NOT EXISTS relationships (
            id TEXT PRIMARY KEY,
            from_id TEXT NOT NULL,
            to_id TEXT NOT NULL,
            type TEXT NOT NULL,
            line INTEGER,
            context TEXT,
            created_at INTEGER NOT NULL,
            FOREIGN KEY (from_id) REFERENCES entities(id) ON DELETE CASCADE,
            FOREIGN KEY (to_id) REFERENCES entities(id) ON DELETE CASCADE
        );

        CREATE INDEX IF NOT EXISTS idx_entities_file_path ON entities(file_path);
        CREATE INDEX IF NOT EXISTS idx_entities_type ON entities(type);
        CREATE INDEX IF NOT EXISTS idx_entities_name ON entities(name);
        CREATE INDEX IF NOT EXISTS idx_rel_from ON relationships(from_id);
        CREATE INDEX IF NOT EXISTS idx_rel_to ON relationships(to_id);
        
        CREATE TABLE IF NOT EXISTS files (
            path TEXT PRIMARY KEY,
            hash TEXT,
            last_indexed INTEGER NOT NULL,
            entity_count INTEGER NOT NULL
        );
        
        PRAGMA foreign_keys = ON;
        PRAGMA journal_mode = WAL;
    )SQL";

    EntityType stringToEntityType(const std::string& str) {
        if (str == "function") return EntityType::Function;
        if (str == "class") return EntityType::Class;
        if (str == "method") return EntityType::Method;
        if (str == "struct") return EntityType::Struct;
        if (str == "variable") return EntityType::Variable;
        if (str == "field") return EntityType::Field;
        if (str == "namespace") return EntityType::Namespace;
        if (str == "enum") return EntityType::Enum;
        if (str == "typedef") return EntityType::Typedef;
        if (str == "template") return EntityType::Template;
        throw std::invalid_argument("Unknown EntityType string: " + str);
    }

    RelationType stringToRelationType(const std::string& str) {
        if (str == "Calls") return RelationType::Calls;
        if (str == "Contains") return RelationType::Contains;
        if (str == "References") return RelationType::References;
        if (str == "Extends") return RelationType::Extends;
        if (str == "Implements") return RelationType::Implements;
        if (str == "Uses") return RelationType::Uses;
        if (str == "Imports") return RelationType::Imports;
        throw std::invalid_argument("Unknown RelationType string: " + str);
    }

    std::string makeEntityId(const Entity& e) {
        std::ostringstream oss;
        oss << e.filePath << "|" << entityTypeToString(e.type)
        << "|" << e.name
        << "|" << e.location.start.line
        << ":" << e.location.start.column
        << "-" << e.location.end.line
        << ":" << e.location.end.column;
        return oss.str();
    }

    
    std::string makeRelationshipId(const Relationship& r) {
        std::ostringstream oss;
        oss << r.fromId << ">" << r.toId << "|" << relationTypeToString(r.type);
        return oss.str();
    }

}
    class SqliteGraphStorage final : public IGraphStorage {
        public:

        explicit SqliteGraphStorage(const std::string& dbPath)
            : db_(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
                db_.exec("PRAGMA foreign_keys = ON;");
        }

        void initialize() override {
            db_.exec(kSchemaV1);
        }

        std::string insertEntity(const Entity& entity) override {
            auto id = makeEntityId(entity);
            SQLite::Statement q(db_,
                "INSERT OR REPLACE INTO entities "
                "(id, name, type, file_path, "
                " start_line, start_column, start_index, "
                " end_line, end_column, end_index, "
                " signature, return_type, modifiers, template_params, base_classes, "
                " hash, created_at, updated_at) "
                "VALUES (?,?,?,?, ?,?,?,?,?,?, ?,?,?,?,?, ?,?,?)");

            q.bind(1, id);
            q.bind(2, entity.name);
            q.bind(3, std::string(entityTypeToString(entity.type)));
            q.bind(4, entity.filePath);
            q.bind(5, entity.location.start.line);
            q.bind(6, entity.location.start.column);
            q.bind(7, entity.location.start.index);
            q.bind(8, entity.location.end.line);
            q.bind(9, entity.location.end.column);
            q.bind(10, entity.location.end.index);

            if (entity.metadata.signature)  q.bind(11, *entity.metadata.signature);
            else q.bind(11);  

            if (entity.metadata.returnType) q.bind(12, *entity.metadata.returnType);
            else q.bind(12);

            q.bind(13, entity.metadata.modifiers ? nlohmann::json(*entity.metadata.modifiers).dump(): "");
            q.bind(14, entity.metadata.templateParams ? nlohmann::json(*entity.metadata.templateParams).dump(): "");
            q.bind(15, entity.metadata.baseClasses ? nlohmann::json(*entity.metadata.baseClasses).dump(): "");
            q.bind(16, entity.hash);
            q.bind(17, nowMillis());
            q.bind(18, nowMillis());

            q.exec();
            return id;
        }
        std::optional<Entity> getEntity(const std::string& id) override {
            SQLite::Statement q(db_, 
                "SELECT id, name, type, file_path, "
                "start_line, start_column, start_index, "
                "end_line, end_column, end_index, "
                "signature, return_type, modifiers, template_params, base_classes, "
                "hash, created_at, updated_at "
                "FROM entities WHERE id = ?");
            q.bind(1, id);
            if (!q.executeStep()) return std::nullopt;
            return rowToEntity(q);
        }

        std::vector<Entity> findEntities(const EntityQuery& query) override {
            std::ostringstream sql;
            sql << "SELECT id, name, type, file_path, "
                << "start_line, start_column, start_index, "
                << "end_line, end_column, end_index, "
                << "signature, return_type, modifiers, template_params, base_classes, "
                << "hash, created_at, updated_at "
                << "FROM entities WHERE 1=1";
            if (query.type)     sql << " AND type = ?";
            if (query.filePath) sql << " AND file_path = ?";
            if (query.name)     sql << " AND name = ?";
            sql << " LIMIT ? OFFSET ?";

            SQLite::Statement q(db_, sql.str());
            int idx = 1;
            if (query.type)     q.bind(idx++, std::string(entityTypeToString(*query.type)));
            if (query.filePath) q.bind(idx++, *query.filePath);
            if (query.name)     q.bind(idx++, *query.name);
            q.bind(idx++, *query.limit);
            q.bind(idx++, *query.offset);

            std::vector<Entity> out;
            while (q.executeStep()) out.push_back(rowToEntity(q));
            return out;
        }

        bool deleteEntity(const std::string& id) override {
            SQLite::Statement q(db_, "DELETE FROM entities WHERE id = ?");
            q.bind(1, id);
            return q.exec() > 0;  
        }

        std::string insertRelationship(const Relationship& r) override {
            auto id = makeRelationshipId(r);
            SQLite::Statement q(db_,
                "INSERT OR REPLACE INTO relationships "
                "(id, from_id, to_id, type, line, context, created_at) "
                "VALUES (?,?,?,?, ?,?,?)");
            q.bind(1, id);
            q.bind(2, r.fromId);
            q.bind(3, r.toId);
            q.bind(4, std::string(relationTypeToString(r.type)));
            if (r.metadata && r.metadata->line) {
                q.bind(5, *r.metadata->line);
            } else {
                q.bind(5);
            }
            if (r.metadata && r.metadata->context) {
                q.bind(6, *r.metadata->context);
            } else {
                q.bind(6);
            }
            q.bind(7, nowMillis());
            q.exec();
            return id;
        }

        std::vector<Relationship> getRelationshipsForEntity(const std::string& entityId) override {
            SQLite::Statement q(db_, "SELECT id, from_id, to_id, type, line, context, created_at FROM relationships WHERE from_id=? OR to_id=?");
            q.bind(1, entityId);
            q.bind(2, entityId);
            std::vector<Relationship> out;
            while (q.executeStep()) out.push_back(rowToRelationship(q));
            return out;
        }

        void upsertFileInfo(const FileInfo& info) override {
            SQLite::Statement q(db_,
                "INSERT OR REPLACE INTO files "
                "(path, hash, last_indexed, entity_count) VALUES (?,?,?,?)");
            q.bind(1, info.path);
            q.bind(2, info.hash);
            q.bind(3, info.lastIndexed);
            q.bind(4, info.entityCount);
            q.exec();
        }

        std::optional<FileInfo> getFileInfo(const std::string& path) override {
            SQLite::Statement q(db_, "SELECT path, hash, last_indexed, entity_count FROM files WHERE path = ?");
            q.bind(1, path);
            if (!q.executeStep()) return std::nullopt;
            
            FileInfo info;
            info.path = q.getColumn("path").getString();
            info.hash = q.getColumn("hash").getString();
            info.lastIndexed = q.getColumn("last_indexed").getInt64();
            info.entityCount = q.getColumn("entity_count").getInt();
            return info;
        }

        Stats getStats() override {
            Stats s;
            SQLite::Statement e(db_, "SELECT COUNT(*) FROM entities");
            e.executeStep(); s.totalEntities = e.getColumn(0).getInt();
            SQLite::Statement r(db_, "SELECT COUNT(*) FROM relationships");
            r.executeStep(); s.totalRelationships = r.getColumn(0).getInt();
            SQLite::Statement f(db_, "SELECT COUNT(*) FROM files");
            f.executeStep(); s.totalFiles = f.getColumn(0).getInt();
            return s;
        }

    private:
        SQLite::Database db_;
 
        Entity rowToEntity(SQLite::Statement& q) {
            Entity e;
            e.id        = q.getColumn("id").getString();
            e.name      = q.getColumn("name").getString();
            e.type      = stringToEntityType(q.getColumn("type").getString());
            e.filePath  = q.getColumn("file_path").getString();
            e.location.start.line    = q.getColumn("start_line").getInt();
            e.location.start.column  = q.getColumn("start_column").getInt();
            e.location.start.index   = q.getColumn("start_index").getInt();
            e.location.end.line      = q.getColumn("end_line").getInt();
            e.location.end.column    = q.getColumn("end_column").getInt();
            e.location.end.index     = q.getColumn("end_index").getInt();

            if (!q.getColumn("signature").isNull())
                e.metadata.signature = q.getColumn("signature").getString();
            if (!q.getColumn("return_type").isNull())
                e.metadata.returnType = q.getColumn("return_type").getString();

            if (!q.getColumn("modifiers").isNull()) {
                std::string mods = q.getColumn("modifiers").getString();
                if (!mods.empty()) e.metadata.modifiers = parseJsonArrayColumn(mods);
            }
            if (!q.getColumn("template_params").isNull()) {
                std::string tparams = q.getColumn("template_params").getString();
                if (!tparams.empty()) e.metadata.templateParams = parseJsonArrayColumn(tparams);
            }
            if (!q.getColumn("base_classes").isNull()) {
                std::string bases = q.getColumn("base_classes").getString();
                if (!bases.empty()) e.metadata.baseClasses = parseJsonArrayColumn(bases);
            }

            e.hash      = q.getColumn("hash").getString();
            e.createdAt = q.getColumn("created_at").getInt64();
            e.updatedAt = q.getColumn("updated_at").getInt64();
            return e;
        }

        Relationship rowToRelationship(SQLite::Statement& q) {
            Relationship r;
            r.id = q.getColumn("id").getString();
            r.fromId = q.getColumn("from_id").getString();
            r.toId = q.getColumn("to_id").getString();
            r.type = stringToRelationType(q.getColumn("type").getString());
            
            if (!q.getColumn("line").isNull() || !q.getColumn("context").isNull()) {
                RelationshipMetadata meta;
                if (!q.getColumn("line").isNull()) meta.line = q.getColumn("line").getInt();
                if (!q.getColumn("context").isNull()) meta.context = q.getColumn("context").getString();
                r.metadata = meta;
            }
            r.createdAt = q.getColumn("created_at").getInt64();
            return r;
        }

        std::vector<std::string> parseJsonArrayColumn(const std::string& json_str) {
            if (json_str.empty()) return {};
            try {
                auto j = nlohmann::json::parse(json_str);
                if (j.is_array()) {
                    return j.get<std::vector<std::string>>();
                }
            } catch (const nlohmann::json::exception&) {
                // Ignore parsing errors for malformed JSON, return empty
            }
            return {};
        }
    };

    std::unique_ptr<IGraphStorage> createSqliteStorage(const std::string& dbPath) {
        auto storage = std::make_unique<SqliteGraphStorage>(dbPath);
        storage->initialize();
        return storage;
    }

}
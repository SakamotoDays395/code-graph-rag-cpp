#include "parser.hpp"

#include <tree_sitter/api.h>

#include <chrono>

extern "C" const TSLanguage* tree_sitter_cpp();

#include <algorithm>
#include <cstring>
#include <fstream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <string>

namespace code_graph {

namespace {
    //RAII WRAPPERS

    struct ParserDeleter {
        void operator()(TSParser* p) const noexcept {
            if (p) ts_parser_delete(p);
        }
    };
    using ParserRef = std::unique_ptr<TSParser, ParserDeleter>;

    struct TreeDeleter {
        void operator()(TSTree* t) const noexcept {
            if (t) ts_tree_delete(t);
        }
    };
    using TreeRef = std::unique_ptr<TSTree, TreeDeleter>;

    Position makePosition(TSPoint p, int index) {
        return Position {
            static_cast<int>(p.row) + 1, 
            static_cast<int>(p.column), 
            index 
        };
    }

    Location makeLocation(TSNode node, const std::string& source) {
        TSPoint sp = ts_node_start_point(node);
        TSPoint ep = ts_node_end_point(node);
        return Location {
            makePosition(sp, static_cast<int>(ts_node_start_byte(node))),
            makePosition(ep, static_cast<int>(ts_node_end_byte(node)))
        };
    }

    std::string nodeText(TSNode node, const std::string& source) {
        auto start = ts_node_start_byte(node);
        auto end = ts_node_end_byte(node);
        if (start < source.size() && end <= source.size() && start <= end) {
            return source.substr(start, end - start);
        }
        return "";
    }

    std::string textOf(TSNode node, const std::string& source) {
        return nodeText(node, source);
    }

    TSNode firstChildOfType(TSNode node, const char* type) {
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_named_child(node, i);
            if (strcmp(ts_node_type(child), type) == 0) return child;
        }
        TSNode null = {0};
        return null;
    }

    std::string resolveName(TSNode node, const std::string& source) {
        static const char* kNameTypes[] = {
            "identifier", "type_identifier", "field_identifier", "qualified_identifier", "namespace_identifier"
        };
        for (const char* t : kNameTypes) {
            TSNode child = firstChildOfType(node, t);
            if(!ts_node_is_null(child)) return textOf(child, source);
        }
        uint32_t n = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < n; i++) {
            TSNode c = ts_node_named_child(node, i);
            std::string r = resolveName(c, source);
            if (!r.empty()) return r;
        }
        return {};
    }

    std::string qualify(const std::string& scope, const std::string& name) {
        if (scope.empty()) return name;
        if (name.empty()) return scope;
        if(name.find("::") != std::string::npos) return name;
        return scope + "::" + name;
    }

    std::string stableHash(const std::string& text) {
        if (text.empty()) return "0";
        std::ostringstream oss;
        oss << std::hex << text.size() << ":" <<text.substr(0, std::min(text.size(), size_t(16)));
        return oss.str();
    }
}//anonymous namespace

    struct Parser::Impl {
        IGraphStorage* storage = nullptr;
        ParserRef parser;
        std::string currentFile;

        int entitiesEmitted = 0;
        int relationshipsEmitted = 0;

        std::string source;

        void reset(const std::string& fpath, const std::string& src) {
            currentFile = fpath;
            entitiesEmitted = 0;
            relationshipsEmitted = 0;
            source = src;
        }

        bool ensureParser() {
            if (parser) return true;
            TSParser* raw = ts_parser_new();
            if (!raw) return false;
            if (!ts_parser_set_language(raw, tree_sitter_cpp())) {
                ts_parser_delete(raw);
                return false;
            }
            parser.reset(raw);
            return true;
        }

        void walk(TSNode node, const std::string& scopeId);
        void emitFunction(TSNode node, const std::string& scopeId);
        std::string emitClass(TSNode node, EntityType kind, const std::string& scopeId);
        std::string emitNamespace(TSNode node, const std::string& scopeId);
        void emitVariable(TSNode node, const std::string& scopeId);
        void extractCalls(TSNode node, const std::string& callerId);
    };

    Parser::Parser(IGraphStorage& storage) : impl_(std::make_unique<Impl>()) {
        impl_->storage = &storage;
        if (!impl_->ensureParser()) {
            throw std::runtime_error("Parser could not be initialized.");
        }
    }

    Parser::~Parser() = default;
    Parser::Parser(Parser&&) noexcept = default;
    Parser& Parser::operator=(Parser&&) noexcept = default;

    ParseResult Parser::parseFile(const std::string& filePath) {
        std::ifstream f(filePath, std::ios::binary);
        if (!f) return ParseResult{ filePath, 0, 0,0,false,"cannot open file"};
        std::ostringstream ss;
        ss << f.rdbuf();
        return parseString(ss.str(), filePath);
    }

    ParseResult Parser::parseString(const std::string& source, const std::string& virtualFilePath) {
        auto t0 = std::chrono::steady_clock::now();
        impl_->reset(virtualFilePath, source);

        if (!impl_->parser) {
            return ParseResult{ virtualFilePath, 0, 0,0,false,"parser not initialized"};
        }

        TSTree* rawTree = ts_parser_parse_string(
            impl_->parser.get(),
            nullptr,
            source.data(),
            static_cast<uint32_t>(source.size()));
        if (!rawTree) {
            return ParseResult{ virtualFilePath, 0, 0,0,false,"tree-sitter parse failed"};
        }
        TreeRef tree(rawTree);
        TSNode root = ts_tree_root_node(tree.get());
        impl_->walk(root, "");
        auto t1 = std::chrono::steady_clock::now();
        int ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
        return ParseResult{virtualFilePath, impl_->entitiesEmitted, impl_->relationshipsEmitted, ms, true, {}};
    }

    void Parser::Impl::walk(TSNode node, const std::string& scopeId) {
        if (ts_node_is_null(node)) return;

        const char* type = ts_node_type(node);

        std::string childScope = scopeId;

        if (std::string(type) == "function_definition") {
            emitFunction(node, scopeId);
        } else if (std::string(type) == "class_specifier") {
            childScope = emitClass(node, EntityType::Class, scopeId);
        } else if (std::string(type) == "struct_specifier") {
            childScope = emitClass(node, EntityType::Struct, scopeId);
        } else if (std::string(type) == "namespace_definition") {
            childScope = emitNamespace(node, scopeId);
        } else if (std::string(type) == "declaration") {
            emitVariable(node, scopeId);
        }

        uint32_t n = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < n; ++i) {
            walk(ts_node_named_child(node, i), childScope);
        }
    }

    void Parser::Impl::emitFunction(TSNode node, const std::string& scopeId) {
        std::string name = resolveName(node, source);
        if (name.empty()) name = "<anonymous>";

        Entity e{};
        e.name = name;
        e.type = EntityType::Function;
        e.filePath = currentFile;
        e.location = makeLocation(node, source);
        e.metadata.signature = nodeText(node, source);
        e.hash = stableHash(*e.metadata.signature);

        std::string id = storage->insertEntity(e);
        entitiesEmitted++;

        if (!scopeId.empty()) {
            Relationship rel{};
            rel.fromId = scopeId;
            rel.toId = id;
            rel.type = RelationType::Contains;
            rel.metadata = RelationshipMetadata{e.location.start.line, {}};
            storage->insertRelationship(rel);
            relationshipsEmitted++;
        }
        extractCalls(node, id);
    }


    void Parser::Impl::extractCalls(TSNode node, const std::string& callerId) {
        if (ts_node_is_null(node)) return;

        // When we find a function call in the code...
        if (std::string(ts_node_type(node)) == "call_expression") {
            // The first child of a call_expression is the name of the function being called
            TSNode funcNode = ts_node_named_child(node, 0); 
            std::string targetName = textOf(funcNode, source);
            
            if (!targetName.empty()) {
                // We don't know the full details of this target function yet, so we make a placeholder
                Entity placeholder{};
                placeholder.name = targetName;
                placeholder.type = EntityType::Function;
                placeholder.filePath = "<unknown>";
                placeholder.location = Location{{0,0,0},{0,0,0}};
                
                std::string targetId = storage->insertEntity(placeholder);

                // Create the 'Calls' relationship edge connecting the caller to the target
                Relationship rel{};
                rel.fromId = callerId;
                rel.toId = targetId;
                rel.type = RelationType::Calls;
                storage->insertRelationship(rel);
                relationshipsEmitted++;
            }
        }

        // Recursively walk through the rest of the function body looking for more calls
        uint32_t n = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < n; ++i) {
            extractCalls(ts_node_named_child(node, i), callerId);
        }
    }

    std::string Parser::Impl::emitClass(TSNode node, EntityType kind, const std::string& scopeId) {
        std::string name = resolveName(node, source);
        if (name.empty()) name = "<anonymous>";

        Entity e{};
        e.name = name;
        e.type = kind;
        e.filePath = currentFile;
        e.location = makeLocation(node, source);
        e.hash = stableHash(nodeText(node, source));

        TSNode bases = firstChildOfType(node, "base_class_clause");
        if (!ts_node_is_null(bases)) {
            std::vector<std::string> baseNames;
            uint32_t bn = ts_node_named_child_count(bases);
            for (uint32_t i = 0; i < bn; ++i) {
                TSNode b = ts_node_named_child(bases, i);
                std::string t = textOf(b, source);
                if (!t.empty()) baseNames.push_back(t);
            }
            if (!baseNames.empty()) e.metadata.baseClasses = baseNames;
        }

        std::string id = storage->insertEntity(e);
        entitiesEmitted++;

        if (!scopeId.empty()) {
            Relationship rel{};
            rel.fromId = scopeId;
            rel.toId   = id;
            rel.type   = RelationType::Contains;
            storage->insertRelationship(rel);
            relationshipsEmitted++;
        }

        if (e.metadata.baseClasses) {
            for (const auto& baseName : *e.metadata.baseClasses) {
                Entity placeholder{};
                placeholder.name = baseName;
                placeholder.type = EntityType::Class;
                placeholder.filePath = "<unknown>";
                placeholder.location = Location{{0,0,0},{0,0,0}};
                std::string baseId = storage->insertEntity(placeholder);

                Relationship rel{};
                rel.fromId = id;
                rel.toId   = baseId;
                rel.type   = RelationType::Extends;
                storage->insertRelationship(rel);
                relationshipsEmitted++;
            }
        }
        return id;
    }

    ParseResult parseFileInto(IGraphStorage& storage, const std::string& filePath) {
        Parser p(storage);
        return p.parseFile(filePath);
    }

    std::string Parser::Impl::emitNamespace(TSNode node, const std::string& scopeId) {
        std::string name = resolveName(node, source);
        if (name.empty()) name = "<anonymous>";

        Entity e{};
        e.name = name;
        e.type = EntityType::Namespace;
        e.filePath = currentFile;
        e.location = makeLocation(node, source);
        e.hash = stableHash(nodeText(node, source));

        std::string id = storage->insertEntity(e);
        entitiesEmitted++;

        if (!scopeId.empty()) {
            Relationship rel{};
            rel.fromId = scopeId;
            rel.toId   = id;
            rel.type   = RelationType::Contains;
            storage->insertRelationship(rel);
            relationshipsEmitted++;
        }
        return id;
    }

    void Parser::Impl::emitVariable(TSNode node, const std::string& scopeId) {
        std::string name = resolveName(node, source);
        if (name.empty()) return;

        Entity e{};
        e.name = name;
        e.type = EntityType::Variable;
        e.filePath = currentFile;
        e.location = makeLocation(node, source);
        e.hash = stableHash(nodeText(node, source));

        std::string id = storage->insertEntity(e);
        entitiesEmitted++;

        if (!scopeId.empty()) {
            Relationship rel{};
            rel.fromId = scopeId;
            rel.toId   = id;
            rel.type   = RelationType::Contains;
            storage->insertRelationship(rel);
            relationshipsEmitted++;
        }
    }

    }  // namespace code_graph

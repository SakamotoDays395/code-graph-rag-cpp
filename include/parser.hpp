#pragma once

#include <string>
#include <string_view>
#include <memory>

#include "storage.hpp"
#include "types.hpp"

namespace code_graph {

    struct ParseResult {
        std::string filePath;
        int entitiesEmitted = 0;
        int relationshipsEmitted = 0;
        int durationMs = 0;
        bool ok = true;
        std::string errorMessage;
    };
   
    class Parser {
        public:
            explicit Parser(IGraphStorage& storage);
            ~Parser();
            // the copy assignment and copy constructor are deleted to avoid more than one parser.
            Parser(const Parser&) = delete;
            Parser& operator=(const Parser&) = delete;

            Parser(Parser&&) noexcept;
            Parser& operator=(Parser&&) noexcept;

            ParseResult parseFile(const std::string& filePath);
            ParseResult parseString(const std::string& source, const std::string& virtualFilePath);
        
        private:
            struct Impl;
            std::unique_ptr<Impl> impl_;
    };

    ParseResult parseFileInto(IGraphStorage& storage, const std::string& filePath);

} //namspace code_graph
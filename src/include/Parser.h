//
// Created by bruno on 12/01/2026.
//

#ifndef CPP_SERIALIZER_PARSER_H
#define CPP_SERIALIZER_PARSER_H

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace serializer {
    // Enum para modificadores de acesso
    enum class AccessSpecifier {
        Public,     // public:
        Private,    // private:
        Protected,  // protected:
        None        // nenhum especificado (struct padrão public)
    };

    struct FieldInfo {
        std::string type;
        std::string name;
        AccessSpecifier access;  // Em qual seção está
        bool isTransient;        // Tem macro TRANSIENT?
    };

    struct ClassInfo {
        std::string name;
        std::vector<FieldInfo> fields;
        std::filesystem::path sourceFile;
        bool isStruct;  // struct (public por padrão) ou class (private por padrão)
    };

    class Parser {
    public:
        std::optional<ClassInfo> parseClass(const std::filesystem::path& filePath) const;
        Parser() = default;

    private:
        AccessSpecifier currentAccessFromLine(const std::string& line) const;
        // bool containsTransient(const std::string& line) const;
        // std::optional<FieldInfo> parseFieldLine(const std::string& line, AccessSpecifier currentAccess) const;
    };
}
#endif //CPP_SERIALIZER_PARSER_H
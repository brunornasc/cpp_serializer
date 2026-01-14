//
// Created by bruno on 12/01/2026.
//

#include "include/TypeChecker.h"
#include "include/Utils.h"

#include <string>
#include <unordered_set>
#include <vector>

namespace serializer {
    TypeChecker::TypeChecker() {
        initializeTypes();
    }

    void TypeChecker::initializeTypes() {
        // Tipos primitivos
        primitiveTypes_ = {
            "int", "int8_t", "int16_t", "int32_t", "int64_t",
            "uint8_t", "uint16_t", "uint32_t", "uint64_t",
            "float", "double", "long double",
            "bool", "char", "wchar_t", "char16_t", "char32_t",
            "short", "long", "long long", "unsigned", "signed",
            "size_t", "ptrdiff_t", "intptr_t", "uintptr_t"
        };

        // Tipos STL básicos
        stlTypes_ = {
            "std::string", "std::string_view",
            "std::byte"
        };

        // Padrões de containers (verificar template args depois)
        containerPatterns_ = {
            "std::vector<",
            "std::list<",
            "std::deque<",
            "std::array<",
            "std::set<",
            "std::unordered_set<",
            "std::map<",
            "std::unordered_map<",
            "std::pair<",
            "std::tuple<",
            "std::optional<",
            "std::variant<"
        };
    }

    bool TypeChecker::isSerializableType(const std::string& typeName) const {
        std::string type = typeName;

        // Remove const, volatile, etc
        const std::vector<std::string> qualifiers = {"const", "volatile", "mutable"};
        for (const auto& q : qualifiers) {
            size_t pos = type.find(q);
            if (pos != std::string::npos &&
                (type[pos + q.length()] == ' ' || type[pos + q.length() == 0])) {
                type.erase(pos, q.length());
            }
        }

        type = Utils::trim(type);

        // É tipo primitivo?
        if (primitiveTypes_.count(type)) return true;

        // É tipo STL básico?
        if (stlTypes_.count(type)) return true;

        // É container?
        if (isContainerType(type)) return true;

        // TODO: Verificar se é enum ou classe serializável

        return false;
    }

    bool TypeChecker::isContainerType(const std::string& typeName) const {
        for (const auto& pattern : containerPatterns_) {
            if (typeName.find(pattern) == 0) { // Começa com o padrão
                return true;
            }
        }
        return false;
    }
}
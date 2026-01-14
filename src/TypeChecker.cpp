// TypeChecker.cpp - implementação
#include "include/TypeChecker.h"
#include "include/Utils.h"
#include <algorithm>
#include <stack>

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

        // Padrões de containers
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

    void TypeChecker::registerSerializableClass(const ClassInfo& classInfo) {
        serializableClasses_.insert(classInfo.name);
        classRegistry_[classInfo.name] = classInfo;
    }

    TypeChecker::TypeAnalysis TypeChecker::analyzeType(const std::string& typeName) const {
        TypeAnalysis analysis;
        std::string cleaned = typeName;

        // Remove const, volatile, etc
        std::vector<std::string> qualifiers = {"const", "volatile", "mutable"};
        for (const auto& q : qualifiers) {
            size_t pos = cleaned.find(q);
            if (pos != std::string::npos) {
                cleaned.erase(pos, q.length());
            }
        }

        cleaned = Utils::trim(cleaned);

        // Verifica se é tipo primitivo
        if (primitiveTypes_.count(cleaned)) {
            analysis.category = TypeCategory::Primitive;
            analysis.baseType = cleaned;
            return analysis;
        }

        // Verifica se é string
        if (cleaned == "std::string" || cleaned == "std::string_view") {
            analysis.category = TypeCategory::String;
            analysis.baseType = cleaned;
            return analysis;
        }

        // Verifica se é container
        auto [base, templateArgs] = Utils::extractTemplateInfo(cleaned);

        static const std::unordered_set<std::string> containerBases = {
            "std::vector", "std::list", "std::deque", "std::array",
            "std::set", "std::unordered_set", "std::multiset",
            "std::map", "std::unordered_map", "std::multimap",
            "std::pair", "std::tuple", "std::optional", "std::variant"
        };

        if (containerBases.count(base)) {
            analysis.category = TypeCategory::Container;
            analysis.baseType = base;
            analysis.templateArgs = templateArgs;

            // Verifica se os tipos dentro do container são serializáveis
            for (const auto& arg : templateArgs) {
                TypeAnalysis argAnalysis = analyzeType(arg);
                if (argAnalysis.category == TypeCategory::Unsupported) {
                    analysis.category = TypeCategory::Unsupported;
                    break;
                }
            }

            return analysis;
        }

        // Verifica se é classe serializável
        if (serializableClasses_.count(cleaned)) {
            analysis.category = TypeCategory::Serializable;
            analysis.baseType = cleaned;

            // Verifica recursão (se essa classe contém a si mesma)
            if (hasCircularDependency(cleaned)) {
                analysis.isRecursive = true;
            }

            return analysis;
        }

        // Verifica ponteiros/smart pointers
        if (cleaned.find('*') != std::string::npos ||
            cleaned.find("std::shared_ptr") == 0 ||
            cleaned.find("std::unique_ptr") == 0 ||
            cleaned.find("std::weak_ptr") == 0) {
            analysis.category = TypeCategory::Pointer;
            analysis.baseType = cleaned;
            return analysis;
        }

        // Tipo não suportado
        analysis.category = TypeCategory::Unsupported;
        return analysis;
    }

    bool TypeChecker::hasCircularDependency(const std::string& className,
                                           int maxDepth) const {
        if (maxDepth <= 0) return false;

        auto it = classRegistry_.find(className);
        if (it == classRegistry_.end()) return false;

        const ClassInfo& classInfo = it->second;

        // Verifica se algum campo é do mesmo tipo (auto-referência direta)
        for (const auto& field : classInfo.fields) {
            TypeAnalysis fieldAnalysis = analyzeType(field.type);

            if (fieldAnalysis.baseType == className) {
                return true; // Auto-referência direta
            }

            // Verifica recursão em containers
            if (fieldAnalysis.category == TypeCategory::Container) {
                for (const auto& templateArg : fieldAnalysis.templateArgs) {
                    if (templateArg == className) {
                        return true; // Container do mesmo tipo
                    }

                    // Verifica recursão indireta
                    if (hasCircularDependency(templateArg, maxDepth - 1)) {
                        return true;
                    }
                }
            }

            // Verifica recursão indireta
            if (fieldAnalysis.category == TypeCategory::Serializable &&
                fieldAnalysis.baseType != className) {

                if (hasCircularDependency(fieldAnalysis.baseType, maxDepth - 1)) {
                    return true;
                }
            }
        }

        return false;
    }
}
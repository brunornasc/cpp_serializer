//
// Created by bruno on 12/01/2026.
//

#ifndef CPP_SERIALIZER_TYPECHECKER_H
#define CPP_SERIALIZER_TYPECHECKER_H
// TypeChecker.h - nova versão
#pragma once

#include <string>
#include <unordered_set>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include "ClassInfo.h"

namespace serializer {
    class TypeChecker {
    public:
        TypeChecker();

        enum class TypeCategory {
            Primitive,      // int, float, bool
            String,         // std::string, std::string_view
            Container,      // std::vector<T>, std::map<K,V>
            Serializable,   // Classe marcada com SERIALIZABLE
            Pointer,        // T*, std::shared_ptr<T>, etc.
            Unsupported     // Não pode serializar
        };

        struct TypeAnalysis {
            TypeCategory category;
            std::string baseType;      // "Usuario", "std::vector", etc.
            std::vector<std::string> templateArgs; // Tipos dentro de <>
            bool isRecursive = false;  // Pode causar recursão infinita?
        };

        TypeAnalysis analyzeType(const std::string& typeName) const;
        bool isSerializableType(const std::string& typeName) const;
        bool isSerializableClass(const std::string& className) const;

        // Registra classes serializáveis conhecidas
        void registerSerializableClass(const ClassInfo& classInfo);
        void clearRegisteredClasses();

        // Detecção de ciclos
        bool hasCircularDependency(const std::string& className,
                                  int maxDepth = 4) const;
        std::pair<std::string, std::vector<std::string>> extractTemplateInfo(const std::string& typeName) const;

    private:
        std::unordered_set<std::string> primitiveTypes_;
        std::unordered_set<std::string> stlTypes_;
        std::unordered_set<std::string> serializableClasses_;
        std::unordered_set<std::string> containerPatterns_;
        std::unordered_map<std::string, ClassInfo> classRegistry_;

        void initializeTypes();
    };
}
#endif //CPP_SERIALIZER_TYPECHECKER_H
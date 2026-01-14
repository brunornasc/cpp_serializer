//
// Created by bruno on 12/01/2026.
//

#ifndef CPP_SERIALIZER_TYPECHECKER_H
#define CPP_SERIALIZER_TYPECHECKER_H
#include <string>
#include <unordered_set>
#include <vector>

namespace serializer {
    class TypeChecker {
    public:
        TypeChecker();

        bool isSerializableType(const std::string& typeName) const;
        bool isContainerType(const std::string& typeName) const;
        std::string extractBaseType(const std::string& typeName) const;

    private:
        std::unordered_set<std::string> primitiveTypes_;
        std::unordered_set<std::string> stlTypes_;
        std::vector<std::string> containerPatterns_;

        void initializeTypes();
    };
}
#endif //CPP_SERIALIZER_TYPECHECKER_H
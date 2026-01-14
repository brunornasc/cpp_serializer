//
// Created by bruno on 14/01/2026.
//

#ifndef CPP_SERIALIZER_CLASSINFO_H
#define CPP_SERIALIZER_CLASSINFO_H

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
#include <set>
#include <optional>

namespace serializer {
    // Enum para modificadores de acesso
    enum class AccessSpecifier {
        Public,     // public:
        Private,    // private:
        Protected,  // protected:
        None        // nenhum especificado (struct padrão public)
    };

    struct FieldInfo {
        std::string type;           // "int", "std::string", "Usuario", "std::vector<Produto>"
        std::string name;           // "id", "nome", "usuario", "produtos"
        AccessSpecifier access;     // Em qual seção está (public/private/protected)
        bool isTransient;           // Tem macro TRANSIENT?

        // Informações adicionais para análise de tipo
        bool isPointer = false;     // É um ponteiro (T*, shared_ptr<T>, etc)?
        bool isContainer = false;   // É um container STL?
        std::vector<std::string> containerElementTypes; // Tipos dentro do container

        // Construtores
        FieldInfo() = default;
        FieldInfo(std::string t, std::string n, AccessSpecifier a, bool trans)
            : type(std::move(t)), name(std::move(n)), access(a), isTransient(trans) {}
    };

    struct BaseClassInfo {
        std::string name;           // Nome da classe base
        std::string access;         // "public", "private", "protected"
        bool isVirtual = false;     // Herança virtual?
    };

    struct MethodInfo {
        std::string returnType;     // Tipo de retorno
        std::string name;           // Nome do método
        std::vector<std::string> parameters; // Lista de parâmetros
        bool isVirtual = false;
        bool isPureVirtual = false;
        bool isConst = false;
    };

    struct ClassInfo {
        // Informações básicas
        std::string name;                    // "Usuario", "Produto"
        std::string qualifiedName;           // Com namespace: "meu::ns::Usuario"
        std::filesystem::path sourceFile;    // Arquivo onde está definida
        bool isStruct = false;               // É struct (true) ou class (false)?

        // Conteúdo da classe
        std::vector<FieldInfo> fields;       // Campos/membros dados
        std::vector<BaseClassInfo> baseClasses; // Classes base (herança)
        std::vector<MethodInfo> methods;     // Métodos (para referência futura)

        // Namespace
        std::vector<std::string> namespaces; // Namespaces aninhados

        // Dependências (para ordenação de geração)
        std::set<std::string> dependencies;  // Classes que esta classe depende
        std::set<std::string> dependents;    // Classes que dependem desta

        // Análise de tipo
        bool hasCircularDependency = false;  // Possui dependência circular?
        int maxNestingDepth = 0;             // Profundidade máxima de aninhamento

        // Metadados de serialização
        bool needsCustomSerialization = false; // Precisa de serialização customizada?
        std::vector<std::string> customSerializers; // Serializadores customizados

        // Informações de template (para classes template)
        bool isTemplate = false;
        std::vector<std::string> templateParameters;

        // Construtores
        ClassInfo() = default;
        explicit ClassInfo(std::string className) : name(std::move(className)) {}

        // Métodos de utilidade
        [[nodiscard]] std::vector<FieldInfo> getSerializableFields() const {
            std::vector<FieldInfo> result;
            for (const auto& field : fields) {
                if (field.access == AccessSpecifier::Public && !field.isTransient) {
                    result.push_back(field);
                }
            }
            return result;
        }

        [[nodiscard]] size_t getSerializableFieldCount() const {
            return std::count_if(fields.begin(), fields.end(),
                [](const FieldInfo& f) {
                    return f.access == AccessSpecifier::Public && !f.isTransient;
                });
        }

        [[nodiscard]] bool hasPublicFields() const {
            return std::any_of(fields.begin(), fields.end(),
                [](const FieldInfo& f) {
                    return f.access == AccessSpecifier::Public;
                });
        }

        [[nodiscard]] bool isEmpty() const {
            return fields.empty() && baseClasses.empty();
        }

        [[nodiscard]] std::string getFullName() const {
            if (qualifiedName.empty()) {
                return name;
            }
            return qualifiedName;
        }

        [[nodiscard]] std::string getNamespacedName() const {
            if (namespaces.empty()) {
                return name;
            }

            std::string result;
            for (const auto& ns : namespaces) {
                result += ns + "::";
            }
            result += name;
            return result;
        }

        // Adiciona uma dependência
        void addDependency(const std::string& className) {
            if (className != name) {  // Não adiciona auto-dependência
                dependencies.insert(className);
            }
        }

        // Adiciona um dependente
        void addDependent(const std::string& className) {
            if (className != name) {  // Não adiciona auto-dependente
                dependents.insert(className);
            }
        }

        // Verifica se depende de uma classe específica
        [[nodiscard]] bool dependsOn(const std::string& className) const {
            return dependencies.find(className) != dependencies.end();
        }

        // Verifica se é dependente de uma classe específica
        [[nodiscard]] bool isDependentOf(const std::string& className) const {
            return dependents.find(className) != dependents.end();
        }

        // Calcula grau de dependência (quantas classes dependem desta)
        [[nodiscard]] size_t getDependencyCount() const {
            return dependencies.size();
        }

        // Calcula grau de dependência reversa
        [[nodiscard]] size_t getDependentCount() const {
            return dependents.size();
        }
    };

    // Funções auxiliares
    inline std::string accessSpecifierToString(AccessSpecifier access) {
        switch (access) {
            case AccessSpecifier::Public: return "public";
            case AccessSpecifier::Private: return "private";
            case AccessSpecifier::Protected: return "protected";
            default: return "none";
        }
    }

    inline AccessSpecifier stringToAccessSpecifier(const std::string& str) {
        if (str == "public:") return AccessSpecifier::Public;
        if (str == "private:") return AccessSpecifier::Private;
        if (str == "protected:") return AccessSpecifier::Protected;
        return AccessSpecifier::None;
    }
}

#endif //CPP_SERIALIZER_CLASSINFO_H
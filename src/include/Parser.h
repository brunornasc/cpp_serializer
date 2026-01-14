//
// Created by bruno on 12/01/2026.
//

#ifndef CPP_SERIALIZER_PARSER_H
#define CPP_SERIALIZER_PARSER_H

#include "ClassInfo.h"
#include "TypeChecker.h"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace serializer {
    class Parser {
    public:
        /**
         * Parseia uma classe do arquivo header
         * @param filePath Caminho do arquivo .h/.hpp
         * @return Informações da classe ou std::nullopt se não encontrar SERIALIZABLE
         */
        [[nodiscard]] std::optional<ClassInfo> parseClass(
            const std::filesystem::path& filePath
        ) const;

        /**
         * Parseia uma classe e analisa suas dependências
         * @param filePath Caminho do arquivo
         * @param typeChecker TypeChecker para análise de tipos
         * @return ClassInfo com dependências preenchidas
         */
        [[nodiscard]] std::optional<ClassInfo> parseClassWithDependencies(
            const std::filesystem::path& filePath,
            TypeChecker& typeChecker
        ) const;

        /**
         * Parseia múltiplas classes de um arquivo (para classes aninhadas)
         * @param filePath Caminho do arquivo
         * @return Lista de classes encontradas
         */
        [[nodiscard]] std::vector<ClassInfo> parseAllClasses(
            const std::filesystem::path& filePath
        ) const;

        /**
         * Extrai namespaces do conteúdo do arquivo
         * @param content Conteúdo do arquivo
         * @return Vetor de namespaces (do mais externo para o mais interno)
         */
        [[nodiscard]] std::vector<std::string> extractNamespaces(
            const std::string& content
        ) const;

        /**
         * Verifica se o arquivo contém a macro SERIALIZABLE
         * @param filePath Caminho do arquivo
         * @return true se contém SERIALIZABLE
         */
        [[nodiscard]] bool containsSerializableMacro(
            const std::filesystem::path& filePath
        ) const;

        /**
         * Extrai informações de template de um tipo
         * @param typeName Nome do tipo (ex: "std::vector<Usuario>")
         * @return Par (nome base, argumentos de template)
         */
        [[nodiscard]] static std::pair<std::string, std::vector<std::string>>
        extractTemplateInfo(const std::string& typeName);

    private:
        // Funções auxiliares internas
        [[nodiscard]] AccessSpecifier currentAccessFromLine(const std::string& line) const;
        [[nodiscard]] bool containsTransient(const std::string& line) const;
        [[nodiscard]] std::optional<FieldInfo> parseFieldLine(
            const std::string& line,
            AccessSpecifier currentAccess
        ) const;

        [[nodiscard]] std::optional<BaseClassInfo> parseBaseClass(
            const std::string& line
        ) const;

        [[nodiscard]] std::string readFile(const std::filesystem::path& filePath) const;
        [[nodiscard]] std::vector<std::string> splitLines(const std::string& content) const;
        [[nodiscard]] std::string removeComments(std::string line) const;
        [[nodiscard]] static std::string trim(const std::string& str);

        // Análise de dependências
        void analyzeFieldDependencies(
            ClassInfo& classInfo,
            const FieldInfo& field,
            TypeChecker& typeChecker
        ) const;
    };
}

#endif //CPP_SERIALIZER_PARSER_H
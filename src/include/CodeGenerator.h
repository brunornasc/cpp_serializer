//
// Created by bruno on 12/01/2026.
//

#ifndef CPP_SERIALIZER_CODEGENERATOR_H
#define CPP_SERIALIZER_CODEGENERATOR_H

#include "ClassInfo.h"
#include "TypeChecker.h"
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace serializer {
    class CodeGenerator {
    public:
        CodeGenerator();

        /**
         * Gera arquivo de implementação com métodos de serialização
         * @param classInfo Informações da classe
         * @param outputDir Diretório de saída
         * @param typeChecker TypeChecker para análise de tipos
         * @return Caminho do arquivo gerado
         */
        [[nodiscard]] std::optional<std::filesystem::path> generateImplFile(
            const ClassInfo& classInfo,
            const std::filesystem::path& outputDir,
            const TypeChecker& typeChecker
        ) const;

        /**
         * Gera serialização para múltiplas classes (com resolução de dependências)
         * @param classes Lista de classes para gerar
         * @param outputDir Diretório de saída
         * @param typeChecker TypeChecker para análise
         * @return Lista de arquivos gerados
         */
        [[nodiscard]] std::vector<std::filesystem::path> generateForMultipleClasses(
            const std::vector<ClassInfo>& classes,
            const std::filesystem::path& outputDir,
            const TypeChecker& typeChecker
        ) const;

        /**
         * Modifica a classe original adicionando declarações
         * @param originalHeader Caminho do header original
         * @param classInfo Informações da classe
         * @return true se modificado com sucesso
         */
        [[nodiscard]] bool modifyOriginalClass(
            const std::filesystem::path& originalHeader,
            const ClassInfo& classInfo
        ) const;

        // Configurações
        void setGenerateJson(bool gen) { generateJson_ = gen; }
        void setGenerateGeneric(bool gen) { generateGeneric_ = gen; }
        void setMaxDepth(int depth) { maxDepth_ = depth; }
        void setIndentSize(int size) { indentSize_ = size; }
        void setGenerateRecursive(bool gen) { generateRecursive_ = gen; }

    private:
        // Geração de conteúdo
        [[nodiscard]] std::string generateMethodDeclarations(
            const ClassInfo& classInfo
        ) const;

        [[nodiscard]] std::string generateImplContent(
            const ClassInfo& classInfo,
            const TypeChecker& typeChecker
        ) const;

        [[nodiscard]] std::string generateSerializeMethod(
            const ClassInfo& classInfo,
            const TypeChecker& typeChecker
        ) const;

        [[nodiscard]] std::string generateDeserializeMethod(
            const ClassInfo& classInfo,
            const TypeChecker& typeChecker
        ) const;

        [[nodiscard]] std::string generateFromJsonMethod(
            const ClassInfo& classInfo,
            const TypeChecker& typeChecker
        ) const;

        [[nodiscard]] std::string generateGenericMethods(
            const ClassInfo& classInfo
        ) const;

        bool needsJsonGet(const std::string &type) const;

        // Serialização de containers complexos
        [[nodiscard]] std::string generateContainerSerialization(
            const FieldInfo& field,
            const TypeChecker& typeChecker
        ) const;

        [[nodiscard]] std::string generateContainerDeserialization(
            const FieldInfo& field,
            const std::string& jsonVar,
            const TypeChecker& typeChecker
        ) const;

        // Serialização de objetos aninhados
        [[nodiscard]] std::string generateNestedObjectSerialization(
            const FieldInfo& field
        ) const;

        [[nodiscard]] std::string generateNestedObjectDeserialization(
            const FieldInfo& field,
            const std::string& jsonVar
        ) const;

        // Criação de includes
        [[nodiscard]] std::string generateIncludeForClass(
            const std::string& className,
            const std::filesystem::path& outputDir
        ) const;

        [[nodiscard]] std::string generateForwardDeclarations(
            const ClassInfo& classInfo,
            const TypeChecker& typeChecker
        ) const;

        // Configurações
        bool generateJson_ = true;
        bool generateGeneric_ = true;
        bool generateRecursive_ = true;
        int maxDepth_ = 4;
        int indentSize_ = 4;
    };
}

#endif //CPP_SERIALIZER_CODEGENERATOR_H
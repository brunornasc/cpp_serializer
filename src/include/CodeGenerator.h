//
// Created by bruno on 12/01/2026.
//

#ifndef CPP_SERIALIZER_CODEGENERATOR_H
#define CPP_SERIALIZER_CODEGENERATOR_H

#include <filesystem>
#include <optional>
#include <string>

#include "Parser.h"

namespace serializer {
    class CodeGenerator {
    public:
        CodeGenerator();

        /**
         * Gera arquivo de implementação com métodos de serialização
         * @param classInfo Informações da classe a ser serializada
         * @param outputDir Diretório onde será salvo o arquivo
         * @return Caminho do arquivo gerado ou std::nullopt em caso de erro
         */
        [[nodiscard]] std::optional<std::filesystem::path> generateImplFile(
            const ClassInfo& classInfo,
            const std::filesystem::path& outputDir
        ) const;

        /**
         * Modifica a classe original adicionando declarações dos métodos de serialização
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
        void setIndentSize(int size) { indentSize_ = size; }

    private:
        // Gera declarações dos métodos para inserir na classe
        [[nodiscard]] std::string generateMethodDeclarations(
            const ClassInfo& classInfo
        ) const;

        // Gera conteúdo do arquivo de implementação
        [[nodiscard]] std::string generateImplContent(
            const ClassInfo& classInfo
        ) const;

        // Gera método serialize() que retorna JSON
        [[nodiscard]] std::string generateSerializeMethod(
            const ClassInfo& classInfo
        ) const;

        // Gera método deserialize() que recebe JSON
        [[nodiscard]] std::string generateDeserializeMethod(
            const ClassInfo& classInfo
        ) const;

        // Gera factory method fromJson()
        [[nodiscard]] std::string generateFromJsonMethod(
            const ClassInfo& classInfo
        ) const;

        // Gera métodos template genéricos (Boost-like)
        [[nodiscard]] std::string generateGenericMethods(
            const ClassInfo& classInfo
        ) const;

        // Gera include do arquivo gerado
        [[nodiscard]] std::string generateIncludeLine(
            const std::filesystem::path& originalHeader,
            const std::filesystem::path& generatedHeader
        ) const;

        // Verifica se um tipo precisa de .get<T>() no JSON
        [[nodiscard]] bool needsJsonGet(const std::string& type) const;

        bool generateJson_ = true;
        bool generateGeneric_ = true;
        int indentSize_ = 4;
    };
}

#endif //CPP_SERIALIZER_CODEGENERATOR_H
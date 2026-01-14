//
// Created by bruno on 12/01/2026.
//

#include "include/Parser.h"
#include "include/Utils.h"
#include <fstream>
#include <algorithm>
#include <iostream>

namespace serializer {
    namespace {
        // Verifica se a linha contém "TRANSIENT"
        bool containsTransient(const std::string& line) {
            return line.find("TRANSIENT") != std::string::npos;
        }

        // Extrai tipo e nome de uma declaração de campo
        // Ex: "int id;" -> tipo="int", nome="id"
        // Ex: "std::vector<double> valores;" -> tipo="std::vector<double>", nome="valores"
        std::optional<std::pair<std::string, std::string>>
        extractTypeAndName(const std::string& line) {
            std::string cleaned = line;

            // Remove TRANSIENT se existir
            size_t transientPos = cleaned.find("TRANSIENT");
            if (transientPos != std::string::npos) {
                cleaned.erase(transientPos, 9); // Tamanho de "TRANSIENT"
            }

            // Remove "mutable", "static", "constexpr", etc
            const std::vector<std::string> modifiers = {
                "mutable", "static", "constexpr", "const", "volatile", "inline"
            };

            for (const auto& mod : modifiers) {
                size_t pos = cleaned.find(mod);
                if (pos != std::string::npos &&
                    (cleaned[pos + mod.length()] == ' ' ||
                     cleaned[pos + mod.length()] == '\t')) {
                    cleaned.erase(pos, mod.length());
                }
            }

            cleaned = Utils::trim(cleaned);

            // Remove ponto e vírgula final
            if (!cleaned.empty() && cleaned.back() == ';') {
                cleaned.pop_back();
            }

            // Encontra o último espaço (separando tipo e nome)
            size_t lastSpace = cleaned.find_last_of(" \t");
            if (lastSpace == std::string::npos) {
                return std::nullopt;
            }

            std::string type = Utils::trim(cleaned.substr(0, lastSpace));
            std::string name = Utils::trim(cleaned.substr(lastSpace + 1));

            // Validação básica
            if (type.empty() || name.empty()) {
                return std::nullopt;
            }

            // Verifica se name é válido (não pode ter parênteses, etc)
            if (name.find('(') != std::string::npos ||
                name.find(')') != std::string::npos ||
                name.find('[') != std::string::npos) {
                return std::nullopt; // Provavelmente é um método, não campo
            }

            return std::make_pair(type, name);
        }
    }

    std::optional<ClassInfo> Parser::parseClass(const std::filesystem::path& filePath) const {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Erro ao abrir: " << filePath << "\n";
            return std::nullopt;
        }

        ClassInfo classInfo;
        classInfo.sourceFile = filePath;
        classInfo.isStruct = false;

        std::string line;
        bool inClass = false;
        bool foundSerializable = false;
        AccessSpecifier currentAccess = AccessSpecifier::Private; // class padrão é private
        bool nextFieldIsTransient = false;

        while (std::getline(file, line)) {
            std::string cleanLine = Utils::removeComments(line);

            // Verifica se encontrou SERIALIZABLE
            if (!foundSerializable && cleanLine.find("SERIALIZABLE") != std::string::npos) {
                // Extrai nome da classe: SERIALIZABLE(Usuario)
                size_t start = cleanLine.find('(');
                size_t end = cleanLine.find(')');

                if (start != std::string::npos && end != std::string::npos && end > start) {
                    classInfo.name = cleanLine.substr(start + 1, end - start - 1);
                    classInfo.name = Utils::trim(classInfo.name);
                    foundSerializable = true;
                }

                // Verifica se é struct ou class
                size_t classPos = cleanLine.find("class");
                size_t structPos = cleanLine.find("struct");

                if (structPos != std::string::npos &&
                    (classPos == std::string::npos || structPos < classPos)) {
                    classInfo.isStruct = true;
                    currentAccess = AccessSpecifier::Public; // structs são public por padrão
                }

                inClass = true;
                continue;
            }

            if (!inClass) continue;

            // Verifica se saiu da classe
            if (cleanLine.find('}') != std::string::npos &&
                cleanLine.find(';') != std::string::npos) {
                break; // Fim da classe: "};"
            }

            // Atualiza modificador de acesso atual
            if (AccessSpecifier spec = currentAccessFromLine(cleanLine); spec != AccessSpecifier::None) {
                currentAccess = spec;
                nextFieldIsTransient = false;
                continue;
            }

            // Se linha vazia, pular
            if (cleanLine.empty()) {
                continue;
            }

            bool currentLineHasTransient = containsTransient(cleanLine);

            // Tentar parsear como campo
            // Campos terminam com ; e não têm parênteses (não são métodos)
            if (cleanLine.find(';') != std::string::npos &&
                cleanLine.find('(') == std::string::npos &&
                cleanLine.find(')') == std::string::npos) {

                bool isTransientField = currentLineHasTransient || nextFieldIsTransient;

                if (auto typeAndName = extractTypeAndName(cleanLine)) {
                    FieldInfo field;
                    field.type = typeAndName->first;
                    field.name = typeAndName->second;
                    field.access = currentAccess;
                    field.isTransient = isTransientField;

                    classInfo.fields.push_back(field);
                }
                nextFieldIsTransient = false;
            } else {
                if (cleanLine == "TRANSIENT") {
                    nextFieldIsTransient = true;
                } else {
                    nextFieldIsTransient = false;
                }
            }
        }

        if (!foundSerializable) {
            return std::nullopt;
        }

        return classInfo;
    }

    AccessSpecifier Parser::currentAccessFromLine(const std::string& line) const {
        std::string trimmed = Utils::trim(line);

        if (trimmed == "public:") {
            return AccessSpecifier::Public;
        }

        if (trimmed == "private:") {
            return AccessSpecifier::Private;
        }

        if (trimmed == "protected:") {
            return AccessSpecifier::Protected;
        }

        return AccessSpecifier::None;
    }
}
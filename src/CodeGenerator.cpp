//
// Created by bruno on 12/01/2026.
//

#include "include/CodeGenerator.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <set>

#include "include/TypeChecker.h"
#include "include/Utils.h"

namespace fs = std::filesystem;

namespace serializer {
    namespace {
        std::string makeIndent(int level, int size = 4) {
            return std::string(level * size, ' ');
        }

        std::string toUpper(const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(),
                          [](unsigned char c) { return std::toupper(c); });
            return result;
        }

        std::string toLower(const std::string& str) {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(),
                          [](unsigned char c) { return std::tolower(c); });
            return result;
        }

        std::string trim(const std::string& str) {
            auto start = str.find_first_not_of(" \t");
            if (start == std::string::npos) return "";

            auto end = str.find_last_not_of(" \t");
            return str.substr(start, end - start + 1);
        }
    }

    CodeGenerator::CodeGenerator() {
        generateJson_ = true;
        generateGeneric_ = true;
        indentSize_ = 4;
    }

    std::optional<fs::path> CodeGenerator::generateImplFile(
        const ClassInfo& classInfo,
        const fs::path& outputDir,
        const TypeChecker& typeChecker
    ) const {
        // Cria diretório de saída se não existir
        std::error_code ec;
        if (!fs::exists(outputDir, ec) && !fs::create_directories(outputDir, ec)) {
            std::cerr << "❌ Erro ao criar diretório: " << outputDir << "\n";
            return std::nullopt;
        }

        // Nome do arquivo: Classe_serialization_impl.h
        std::string filename = classInfo.name + "_serialization_impl.h";
        fs::path outputPath = outputDir / filename;

        // Gera conteúdo do arquivo
        std::string content = generateImplContent(classInfo, typeChecker);

        // Escreve no arquivo
        std::ofstream file(outputPath);
        if (!file.is_open()) {
            std::cerr << "❌ Erro ao criar arquivo: " << outputPath << "\n";
            return std::nullopt;
        }

        file << content;
        file.close();

        std::cout << "   ✅ Gerado: " << filename << "\n";
        return outputPath;
    }

    bool CodeGenerator::modifyOriginalClass(
        const fs::path& originalHeader,
        const ClassInfo& classInfo
    ) const {
        // Lê conteúdo original
        std::ifstream inFile(originalHeader);
        if (!inFile.is_open()) {
            std::cerr << "❌ Erro ao abrir: " << originalHeader << "\n";
            return false;
        }

        std::stringstream buffer;
        buffer << inFile.rdbuf();
        std::string content = buffer.str();
        inFile.close();

        // Verifica se já foi modificado (contém serialize())
        if (content.find("serialize()") != std::string::npos ||
            content.find("deserialize(") != std::string::npos) {
            std::cout << "   ⏭️  Classe já modificada, pulando...\n";
            return true; // Já tem os métodos
        }

        // Encontra o fim da classe (procura por "};" após a classe)
        size_t classStart = content.find("class " + classInfo.name);
        if (classStart == std::string::npos) {
            // Talvez seja struct
            classStart = content.find("struct " + classInfo.name);
            if (classStart == std::string::npos) {
                std::cerr << "❌ Não encontrou classe/struct: " << classInfo.name << "\n";
                return false;
            }
        }

        // Encontra o "};" que fecha a classe
        size_t braceCount = 0;
        bool inClass = false;
        size_t classEnd = std::string::npos;

        for (size_t i = classStart; i < content.size(); i++) {
            if (content[i] == '{') {
                braceCount++;
                inClass = true;
            } else if (content[i] == '}') {
                braceCount--;
                if (inClass && braceCount == 0) {
                    // Encontrou o } de fechamento, agora procura o ;
                    for (size_t j = i + 1; j < content.size(); j++) {
                        if (content[j] == ';') {
                            classEnd = j;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        if (classEnd == std::string::npos) {
            std::cerr << "❌ Não encontrou fim da classe\n";
            return false;
        }

        // Encontra última chave antes do fechamento
        size_t lastBrace = content.rfind('}', classEnd - 1);
        if (lastBrace == std::string::npos) {
            std::cerr << "❌ Estrutura da classe inválida\n";
            return false;
        }

        // Gera declarações dos métodos
        std::string declarations = generateMethodDeclarations(classInfo);

        // Insere declarações antes do último "}" (dentro da classe)
        std::string toInsert = "\n" + makeIndent(1, indentSize_) + "// Serialização\n";
        std::istringstream declStream(declarations);
        std::string line;

        while (std::getline(declStream, line)) {
            if (!line.empty()) {
                toInsert += makeIndent(1, indentSize_) + line + "\n";
            }
        }

        content.insert(lastBrace, toInsert);

        // Atualiza posições após inserção
        classEnd += toInsert.length();
        lastBrace += toInsert.length();

        // Salva o arquivo modificado
        std::ofstream outFile(originalHeader);
        if (!outFile.is_open()) {
            std::cerr << "❌ Erro ao salvar: " << originalHeader << "\n";
            return false;
        }

        outFile << content;
        outFile.close();

        std::cout << "   ✅ Métodos adicionados à classe\n";
        return true;
    }

    std::string CodeGenerator::generateMethodDeclarations(
        const ClassInfo& classInfo
    ) const {
        std::stringstream ss;
        int indent = 0;

        // Método serialize() - retorna JSON
        ss << "// Serializa para JSON\n";
        ss << "[[nodiscard]] nlohmann::json serialize() const;\n\n";

        // Método deserialize() - modifica instância
        ss << "// Desserializa de JSON\n";
        ss << "void deserialize(const nlohmann::json& json);\n\n";

        // Factory method
        ss << "// Cria instância a partir de JSON\n";
        ss << "[[nodiscard]] static " << classInfo.name
           << " fromJson(const nlohmann::json& json);\n";

        // Métodos genéricos (se habilitados)
        if (generateGeneric_) {
            ss << "\n";
            ss << "// Serialização genérica (Boost compatible)\n";
            ss << "template<typename Archive>\n";
            ss << "void serialize(Archive& ar) const;\n\n";

            ss << "template<typename Archive>\n";
            ss << "void deserialize(Archive& ar);\n";
        }

        return ss.str();
    }

    std::string CodeGenerator::generateImplContent(
        const ClassInfo& classInfo,
        const TypeChecker& typeChecker
    ) const {
        std::stringstream ss;

        // Header guard
        std::string guardName = toUpper(classInfo.name) + "_SERIALIZATION_IMPL_H";

        ss << "// Arquivo gerado automaticamente por cpp-serializer-gen\n";
        ss << "// Não edite manualmente - será sobrescrito\n\n";

        ss << "#ifndef " << guardName << "\n";
        ss << "#define " << guardName << "\n\n";

        // Includes necessários
        // Inclui o arquivo original da classe
        ss << "#include \"" << classInfo.sourceFile.filename().string() << "\"\n\n";

        // Inclui dependências necessárias
        if (!classInfo.dependencies.empty()) {
            ss << "// Includes para classes dependentes\n";
            for (const auto& dep : classInfo.dependencies) {
                // Gera include para a classe dependente (se ela estiver no mesmo diretório)
                ss << generateIncludeForClass(dep, classInfo.sourceFile.parent_path()) << "\n";
            }
            ss << "\n";
        }

        // Includes STL e bibliotecas
        ss << "// STL includes\n";
        ss << "#include <vector>\n";
        ss << "#include <string>\n";
        ss << "#include <map>\n";
        ss << "#include <optional>\n";
        ss << "#include <variant>\n";
        ss << "#include <memory>\n";  // Para smart pointers
        ss << "\n";

        if (generateJson_) {
            ss << "#include <nlohmann/json.hpp>\n\n";
        }

        // Forward declarations se necessário
        std::string forwardDecls = generateForwardDeclarations(classInfo, typeChecker);
        if (!forwardDecls.empty()) {
            ss << "// Forward declarations\n";
            ss << forwardDecls << "\n";
        }

        ss << "// Implementações de serialização para: " << classInfo.name << "\n\n";

        // Implementação do método serialize()
        if (generateJson_) {
            ss << generateSerializeMethod(classInfo, typeChecker) << "\n\n";
        }

        // Implementação do método deserialize()
        if (generateJson_) {
            ss << generateDeserializeMethod(classInfo, typeChecker) << "\n\n";
        }

        // Implementação do factory method fromJson()
        if (generateJson_) {
            ss << generateFromJsonMethod(classInfo, typeChecker) << "\n\n";
        }

        // Implementação dos métodos genéricos
        if (generateGeneric_) {
            ss << generateGenericMethods(classInfo) << "\n";
        }

        ss << "#endif // " << guardName << "\n";

        return ss.str();
    }

    std::string CodeGenerator::generateSerializeMethod(
        const ClassInfo& classInfo,
        const TypeChecker& typeChecker
    ) const {
        std::stringstream ss;

        ss << "inline nlohmann::json " << classInfo.name << "::serialize() const {\n";
        ss << "    return nlohmann::json{\n";

        bool first = true;
        for (const auto& field : classInfo.fields) {
            if (field.access == AccessSpecifier::Public && !field.isTransient) {
                auto analysis = typeChecker.analyzeType(field.type);

                if (!first) ss << ",\n";

                if (analysis.category == TypeChecker::TypeCategory::Serializable) {
                    // Chama serialize() do objeto aninhado
                    ss << "        {\"" << field.name << "\", "
                       << field.name << ".serialize()}";
                }
                else if (analysis.category == TypeChecker::TypeCategory::Container) {
                    // Container de tipos básicos ou serializáveis
                    ss << "        {\"" << field.name << "\", " << field.name << "}";
                }
                else {
                    // Tipo primitivo ou string
                    ss << "        {\"" << field.name << "\", " << field.name << "}";
                }

                first = false;
            }
        }

        if (first) ss << "        {}";
        ss << "\n    };\n";
        ss << "}\n";

        return ss.str();
    }

    std::string CodeGenerator::generateDeserializeMethod(
        const ClassInfo& classInfo,
        const TypeChecker& typeChecker
    ) const {
        std::stringstream ss;

        ss << "inline void " << classInfo.name << "::deserialize(const nlohmann::json& json) {\n";

        for (const auto& field : classInfo.fields) {
            if (field.access == AccessSpecifier::Public && !field.isTransient) {
                auto analysis = typeChecker.analyzeType(field.type);

                ss << "    " << field.name << " = ";

                if (analysis.category == TypeChecker::TypeCategory::Serializable) {
                    // Desserializa objeto aninhado
                    ss << field.type << "::fromJson(json[\"" << field.name << "\"])";
                }
                else if (analysis.category == TypeChecker::TypeCategory::Container) {
                    // Container - verifica se contém tipos serializáveis
                    bool hasSerializableArgs = false;
                    for (const auto& arg : analysis.templateArgs) {
                        auto argAnalysis = typeChecker.analyzeType(arg);
                        if (argAnalysis.category == TypeChecker::TypeCategory::Serializable) {
                            hasSerializableArgs = true;
                            break;
                        }
                    }

                    if (hasSerializableArgs) {
                        // Container de objetos serializáveis - precisa de desserialização customizada
                        ss << generateContainerDeserialization(field, "json", typeChecker);
                    } else {
                        // Container de tipos básicos
                        ss << "json[\"" << field.name << "\"].get<" << field.type << ">()";
                    }
                }
                else {
                    // Tipo básico
                    ss << "json[\"" << field.name << "\"]";

                    if (needsJsonGet(field.type)) {
                        ss << ".get<" << field.type << ">()";
                    }
                }

                ss << ";\n";
            }
        }

        ss << "}\n";

        return ss.str();
    }

    std::string CodeGenerator::generateContainerDeserialization(
        const FieldInfo& field,
        const std::string& jsonVar,
        const TypeChecker& typeChecker
    ) const {
        auto [base, templateArgs] = Utils::extractTemplateInfo(field.type);

        std::stringstream ss;

        if (base == "std::vector" || base == "std::list" || base == "std::deque") {
            // Vector/list/deque de objetos serializáveis
            std::string elementType = templateArgs[0];
            auto elementAnalysis = typeChecker.analyzeType(elementType);

            ss << "[]() -> " << field.type << " {\n";
            ss << "    " << field.type << " result;\n";
            ss << "    const auto& jsonArray = " << jsonVar << "[\"" << field.name << "\"];\n";
            ss << "    for (const auto& item : jsonArray) {\n";

            if (elementAnalysis.category == TypeChecker::TypeCategory::Serializable) {
                ss << "        result.push_back(" << elementType << "::fromJson(item));\n";
            } else {
                ss << "        result.push_back(item.get<" << elementType << ">());\n";
            }

            ss << "    }\n";
            ss << "    return result;\n";
            ss << "}()";

        } else if (base == "std::map" || base == "std::unordered_map") {
            // Map com objetos serializáveis como valor
            std::string keyType = templateArgs[0];
            std::string valueType = templateArgs[1];
            auto valueAnalysis = typeChecker.analyzeType(valueType);

            ss << "[]() -> " << field.type << " {\n";
            ss << "    " << field.type << " result;\n";
            ss << "    const auto& jsonObject = " << jsonVar << "[\"" << field.name << "\"];\n";
            ss << "    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {\n";
            ss << "        " << keyType << " key = it.key();\n";

            if (valueAnalysis.category == TypeChecker::TypeCategory::Serializable) {
                ss << "        " << valueType << " value = " << valueType << "::fromJson(it.value());\n";
            } else {
                ss << "        " << valueType << " value = it.value().get<" << valueType << ">();\n";
            }

            ss << "        result[key] = value;\n";
            ss << "    }\n";
            ss << "    return result;\n";
            ss << "}()";

        } else {
            // Outros containers - fallback para get<>
            ss << jsonVar << "[\"" << field.name << "\"].get<" << field.type << ">()";
        }

        return ss.str();
    }

    std::string CodeGenerator::generateFromJsonMethod(
        const ClassInfo& classInfo,
        const TypeChecker& typeChecker
    ) const {
        std::stringstream ss;

        ss << "inline " << classInfo.name << " " << classInfo.name
           << "::fromJson(const nlohmann::json& json) {\n";
        ss << "    " << classInfo.name << " obj;\n";
        ss << "    obj.deserialize(json);\n";
        ss << "    return obj;\n";
        ss << "}\n";

        return ss.str();
    }

    std::string CodeGenerator::generateGenericMethods(
        const ClassInfo& classInfo
    ) const {
        std::stringstream ss;

        ss << "// Serialização genérica (compatível com Boost)\n";
        ss << "template<typename Archive>\n";
        ss << "inline void " << classInfo.name << "::serialize(Archive& ar) const {\n";

        for (const auto& field : classInfo.getSerializableFields()) {
            ss << "    ar & " << field.name << ";\n";
        }

        ss << "}\n\n";

        ss << "template<typename Archive>\n";
        ss << "inline void " << classInfo.name << "::deserialize(Archive& ar) {\n";

        for (const auto& field : classInfo.getSerializableFields()) {
            ss << "    ar & " << field.name << ";\n";
        }

        ss << "}\n";

        return ss.str();
    }

    bool CodeGenerator::needsJsonGet(const std::string& type) const {
        // Tipos que precisam de .get<T>() no nlohmann/json
        static const std::set<std::string> needsGetTypes = {
            "std::vector", "std::list", "std::deque", "std::array",
            "std::set", "std::unordered_set", "std::multiset",
            "std::map", "std::unordered_map", "std::multimap",
            "std::pair", "std::tuple", "std::optional", "std::variant"
        };

        std::string lowerType = toLower(type);
        for (const auto& pattern : needsGetTypes) {
            if (lowerType.find(toLower(pattern)) == 0) {
                return true;
            }
        }

        return false;
    }

    std::string CodeGenerator::generateIncludeForClass(
        const std::string& className,
        const std::filesystem::path& baseDir
    ) const {
        // Simplificação: assume que a classe dependente está no mesmo diretório
        // Em uma implementação real, você precisaria mapear nomes de classe para arquivos
        std::string headerName = className + ".h";

        // Verifica se o arquivo existe no mesmo diretório
        std::filesystem::path headerPath = baseDir / headerName;
        if (std::filesystem::exists(headerPath)) {
            return "#include \"" + headerName + "\"";
        }

        // Se não encontrou, tenta com _serialization_impl.h
        headerName = className + "_serialization_impl.h";
        headerPath = baseDir / headerName;
        if (std::filesystem::exists(headerPath)) {
            return "#include \"" + headerName + "\"";
        }

        // Se ainda não encontrou, retorna include genérico
        return "// #include \"" + className + ".h\"  // AUTO-GENERATED: Verifique o caminho correto";
    }

    std::string CodeGenerator::generateForwardDeclarations(
        const ClassInfo& classInfo,
        const TypeChecker& typeChecker
    ) const {
        std::stringstream ss;
        std::set<std::string> forwardDecls;

        // Coleta classes que precisam de forward declaration
        for (const auto& field : classInfo.fields) {
            if (field.access == AccessSpecifier::Public && !field.isTransient) {
                auto analysis = typeChecker.analyzeType(field.type);

                if (analysis.category == TypeChecker::TypeCategory::Serializable) {
                    // Se é um ponteiro ou referência, pode precisar de forward declaration
                    if (field.isPointer) {
                        forwardDecls.insert("class " + analysis.baseType + ";");
                    }
                }

                // Verifica containers
                if (analysis.category == TypeChecker::TypeCategory::Container) {
                    for (const auto& elementType : analysis.templateArgs) {
                        auto elemAnalysis = typeChecker.analyzeType(elementType);
                        if (elemAnalysis.category == TypeChecker::TypeCategory::Serializable &&
                            field.isPointer) {
                            forwardDecls.insert("class " + elemAnalysis.baseType + ";");
                        }
                    }
                }
            }
        }

        // Gera as forward declarations
        for (const auto& decl : forwardDecls) {
            ss << decl << "\n";
        }

        return ss.str();
    }
}
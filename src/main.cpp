#include <iostream>
#include <filesystem>
#include <functional>

#include "include/CodeGenerator.h"
#include "include/FileWalker.h"
#include "include/Parser.h"
#include "include/TypeChecker.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho-do-projeto>\n";
        std::cerr << "Exemplo: " << argv[0] << " ./meu_projeto\n";
        return 1;
    }

    fs::path projectPath = argv[1];

    // Verifica se o caminho existe
    if (!fs::exists(projectPath)) {
        std::cerr << "❌ Caminho não existe: " << projectPath << "\n";
        return 1;
    }

    std::cout << "🚀 Iniciando cpp-serializer-gen\n";
    std::cout << "📁 Projeto: " << fs::absolute(projectPath) << "\n\n";

    // Inicializa componentes
    serializer::FileWalker walker;
    serializer::Parser parser;
    serializer::TypeChecker typeChecker;
    serializer::CodeGenerator generator;

    // Configura generator
    generator.setGenerateJson(true);
    generator.setGenerateGeneric(true);
    generator.setIndentSize(4);

    // Encontra headers
    std::cout << "🔍 Procurando arquivos header...\n";
    auto headers = walker.findHeaderFiles(projectPath);

    if (headers.empty()) {
        std::cout << "⚠️  Nenhum arquivo header encontrado\n";
        return 0;
    }

    std::cout << "📄 Encontrados " << headers.size() << " headers\n\n";

    // Diretório para arquivos gerados
    fs::path generatedDir = projectPath / "generated_serializers";

    // Lista para armazenar todas as classes encontradas
    std::vector<serializer::ClassInfo> allClasses;
    std::unordered_map<std::string, serializer::ClassInfo> classMap;
    std::unordered_map<std::string, fs::path> classToFileMap;

    // Primeira passagem: parse todas as classes e registra no TypeChecker
    std::cout << "📊 Analisando classes...\n";
    for (const auto& header : headers) {
        if (parser.containsSerializableMacro(header)) {
            std::cout << "  📄 " << header.filename() << "\n";

            auto classInfo = parser.parseClass(header);
            if (!classInfo || classInfo.value().fields.empty()) {
                continue;
            }

            // Registra no TypeChecker
            typeChecker.registerSerializableClass(*classInfo);

            // Armazena
            allClasses.push_back(*classInfo);
            classMap[classInfo->name] = *classInfo;
            classToFileMap[classInfo->name] = header;

            std::cout << "    ✨ " << classInfo->name
                      << " (" << classInfo->getSerializableFieldCount()
                      << " campos serializáveis)\n";
        }
    }

    if (allClasses.empty()) {
        std::cout << "\n⚠️  Nenhuma classe com SERIALIZABLE encontrada\n";
        return 0;
    }

    std::cout << "\n🔗 Analisando dependências...\n";

    // Segunda passagem: analisa dependências com TypeChecker atualizado
    for (auto& classInfo : allClasses) {
        // Re-parse com análise de dependências
        auto updatedInfo = parser.parseClassWithDependencies(
            classToFileMap[classInfo.name],
            typeChecker
        );

        if (updatedInfo) {
            classInfo = *updatedInfo;

            if (!classInfo.dependencies.empty()) {
                std::cout << "  📦 " << classInfo.name << " depende de: ";
                for (const auto& dep : classInfo.dependencies) {
                    std::cout << dep << " ";
                }
                std::cout << "\n";
            }
        }
    }

    // Ordenação topológica simples (para evitar dependências circulares)
    std::cout << "\n⚙️  Ordenando por dependências...\n";
    std::vector<serializer::ClassInfo> orderedClasses;
    std::unordered_set<std::string> generated;

    // Função auxiliar para ordenação
    std::function<void(const serializer::ClassInfo&)> processClass;
    processClass = [&](const serializer::ClassInfo& classInfo) {
        if (generated.count(classInfo.name)) return;

        // Primeiro processa dependências
        for (const auto& dep : classInfo.dependencies) {
            if (classMap.count(dep) && !generated.count(dep)) {
                processClass(classMap[dep]);
            }
        }

        // Depois processa esta classe
        orderedClasses.push_back(classInfo);
        generated.insert(classInfo.name);

        std::cout << "  " << (orderedClasses.size()) << ". "
                  << classInfo.name << "\n";
    };

    for (const auto& classInfo : allClasses) {
        processClass(classInfo);
    }

    // Gera serialização
    std::cout << "\n🚀 Gerando serialização...\n";
    int processed = 0;
    int errors = 0;

    for (const auto& classInfo : orderedClasses) {
        std::cout << "\n📄 Processando: " << classInfo.name << "\n";

        // Gera arquivo de implementação
        auto implFile = generator.generateImplFile(
            classInfo,
            generatedDir,
            typeChecker
        );

        if (!implFile) {
            std::cout << "   ❌ Falha ao gerar implementação\n";
            errors++;
            continue;
        }

        // Modifica classe original
        auto originalFile = classToFileMap[classInfo.name];
        if (generator.modifyOriginalClass(originalFile, classInfo)) {
            processed++;
            std::cout << "   ✅ Sucesso!\n";
        } else {
            errors++;
            std::cout << "   ❌ Falha ao modificar classe\n";
        }
    }

    // Resumo
    std::cout << "\n" << std::string(40, '=') << "\n";
    std::cout << "📊 Resultado Final:\n";
    std::cout << "   ✅ Processadas: " << processed << "\n";
    std::cout << "   ❌ Erros: " << errors << "\n";
    std::cout << "   📁 Total de classes: " << allClasses.size() << "\n";

    if (processed > 0) {
        std::cout << "\n🎉 Serialização gerada com sucesso!\n";
        std::cout << "📁 Arquivos gerados em: " << generatedDir << "\n";
        std::cout << "\n💡 Como usar nas suas classes:\n";
        std::cout << "   #include \"sua_classe.h\"\n";
        std::cout << "   \n";
        std::cout << "   SuaClasse obj;\n";
        std::cout << "   nlohmann::json json = obj.serialize();  // Para JSON\n";
        std::cout << "   obj.deserialize(json);                 // De JSON\n";
        std::cout << "   auto novo = SuaClasse::fromJson(json); // Factory method\n";
    }

    return errors > 0 ? 1 : 0;
}

/*
 * Qual a ideia:
 * 1 - Recebe o path do source do projeto por argv ✓
 * 2 - Percorre as pastas em busca do .h ou .hpp ✓
 * 3 - Se tiver o preprocessor SERIALIZABLE começa a fazer a mágica ✓
 * 4 - Vai percorrer o arquivo mapeando tipos inteiros e objetos STL ✓
 * 5 - Vai criar um arquivo e colocar seu include no cabeçalho desse cidadão ✓
 * 6 - nesse arquivo vai implementar 'serialize' e 'deserialize' ✓
 * *** No futuro quem sabe mapear pra criar reflection, seria fixe ***
 * 7 - Vai ignorar variáveis marcadas com 'TRANSIENT' ✓
 * 8 - SOMENTE variáveis públicas serão serializadas ✓
 *
 * Bônus implementado:
 * 9 - Suporte a objetos aninhados serializáveis ✓
 * 10 - Containers de objetos serializáveis ✓
 * 11 - Detecção de dependências cíclicas ✓
 * 12 - Ordenação por dependências ✓
 */
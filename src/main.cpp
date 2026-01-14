#include <iostream>

#include "include/CodeGenerator.h"
#include "include/FileWalker.h"
#include "include/Parser.h"

int main(const int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    // Encontrar todos os arquivos
    auto headers = serializer::FileWalker::findHeaderFiles(argv[1]);
    serializer::CodeGenerator generator;

    // Configura generator
    generator.setGenerateJson(true);
    generator.setGenerateGeneric(true);
    generator.setIndentSize(4);

    std::filesystem::path projectPath = argv[1];
    std::filesystem::path generatedDir = projectPath / "generated_serializers";

    int processed = 0;

    for (const auto& header : headers) {
        auto parser { serializer::Parser() };

        auto classInfo = parser.parseClass(header);

        if (!classInfo.has_value() || classInfo->fields.empty())
            continue;

        auto implFile = generator.generateImplFile(*classInfo, generatedDir);
        if (!implFile) {
            std::cout << "   ❌ Falha ao gerar implementação\n";
            continue;
        }

        // Modifica classe original
        if (generator.modifyOriginalClass(header, *classInfo)) {
            processed++;
            std::cout << "   ✅ Sucesso!\n";

        } else {
            std::cout << "   ❌ Falha ao modificar classe\n";

        }

        processed++;
    }

    /*
     * Qual a ideia:
     * 1 - Recebe o path do source do projeto por argv
     * 2 - Percorre as pastas em busca do .h ou .hpp
     * 3 - Se tiver o preprocessor SERIALIZABLE começa a fazer a mágica
     * 4 - Vai percorrer o arquivo mapeando tipos inteiros e objetos STL
     * 5 - Vai criar um arquivo e colocar seu include no cabeçalho desse cidadão
     * 6 - nesse arquivo vai implementar 'serialize' e 'deserialize'
     * *** No futuro quem sabe mapear pra criar reflection, seria fixe ***
     * 7 - Vai ignorar variáveis marcadas com 'TRANSIENT'
     * 8 - SOMENTE variáveis públicas serão serializadas
     */



    return 0;
}

/*
int main(int argc, char* argv[]) {
if (argc != 2) {
std::cerr << "Uso: " << argv[0] << " <caminho-do-projeto>\n";
return 1;
}

fs::path projectPath = argv[1];
if (!fs::exists(projectPath)) {
std::cerr << "Erro: Caminho não existe: " << projectPath << "\n";
return 1;
}

std::cout << "🔍 Analisando projeto: " << projectPath << "\n";

// Passo 2: Encontrar todos arquivos .h/.hpp
FileWalker walker;
auto headerFiles = walker.findHeaderFiles(projectPath);

std::cout << "📁 Encontrados " << headerFiles.size() << " arquivos header\n";

// Processar cada arquivo
for (const auto& headerFile : headerFiles) {
std::cout << "  📄 Processando: " << headerFile.filename() << "\n";

// Passo 3: Verificar se tem SERIALIZABLE
Parser parser;
if (parser.containsSerializableMacro(headerFile)) {
std::cout << "    ✨ Contém SERIALIZABLE!\n";

// Passo 4: Extrair informações da classe
auto classInfo = parser.parseClass(headerFile);

if (classInfo) {
std::cout << "    📋 Classe: " << classInfo->name
<< " com " << classInfo->fields.size() << " campos\n";

// Passo 5-6: Gerar arquivo de serialização
CodeGenerator generator;
auto generatedFile = generator.generateSerializationFile(
*classInfo,
projectPath
);

if (generatedFile) {
std::cout << "    ✅ Gerado: " << *generatedFile << "\n";

// Adicionar include no header original (opcional)
generator.addIncludeToHeader(headerFile, *generatedFile);
}
}
}
}

std::cout << "🎉 Concluído!\n";
return 0;
}
*/
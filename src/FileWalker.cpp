//
// Created by bruno on 12/01/2026.
//
#include <algorithm>
#include <array>
#include <cctype>
#include <iostream>

#include "include/FileWalker.h"

namespace fs = std::filesystem;

namespace serializer {
    namespace {
        // Função helper local para case-insensitive
        std::string toLower(std::string str) {
            std::ranges::transform(str, str.begin(),
                                   [](const unsigned char c) { return std::tolower(c); });
            return str;
        }

        // Lista de extensões de header
        constexpr std::array<const char*, 5> HEADER_EXTENSIONS = {
            ".h", ".hpp", ".hxx", ".hh", ".h++"
        };

        // Diretórios para ignorar (case-insensitive)
        constexpr std::array<const char*, 8> IGNORE_DIRECTORIES = {
            ".svn", ".git", "node_modules", ".vscode", ".idea",
            "build", "cmake-build-", "__pycache__"
        };

        // Verifica se um diretório deve ser ignorado
        bool shouldSkipDirectory(const fs::path& dirPath) {
            std::string dirName = toLower(dirPath.filename().string());

            // Verifica nomes exatos
            for (const auto& ignoreDir : IGNORE_DIRECTORIES) {
                if (dirName == ignoreDir) {
                    return true;
                }

                // Para "cmake-build-*" (prefix match)
                if (std::string(ignoreDir).find("cmake-build-") == 0 &&
                    dirName.find("cmake-build-") == 0) {
                    return true;
                }
            }

            // Ignorar diretórios começando com ponto (exceto . e ..)
            if (!dirName.empty() && dirName[0] == '.' &&
                dirName != "." && dirName != "..") {
                return true;
            }

            return false;
        }
    }

    std::vector<fs::path> FileWalker::findHeaderFiles(const fs::path& projectPath) {
        std::vector<fs::path> headers;

        // Validação do path de entrada
        std::error_code ec;
        if (!fs::exists(projectPath, ec)) {
            std::cerr << "Erro: Caminho não existe: " << projectPath
                      << " (" << ec.message() << ")\n";
            return headers;
        }

        if (!fs::is_directory(projectPath, ec)) {
            std::cerr << "Erro: " << projectPath << " não é um diretório\n";
            return headers;
        }

        try {
            size_t filesChecked = 0;
            size_t directoriesSkipped = 0;

            // Opção 1: Com skip manual (mais controle)
            for (const auto& entry : fs::recursive_directory_iterator(
                projectPath,
                fs::directory_options::skip_permission_denied
            )) {
                filesChecked++;

                // Se for diretório e estiver na lista de skip
                if (entry.is_directory() && shouldSkipDirectory(entry.path())) {
                    directoriesSkipped++;
                    continue;
                }

                if (entry.is_regular_file() && isHeaderFile(entry.path())) {
                    headers.push_back(entry.path());
                }
            }

            // std::cout << "📊 Total: " << headers.size()
            //           << " headers encontrados em " << filesChecked << " arquivos\n";
            // std::cout << "⏭️  Diretórios ignorados: " << directoriesSkipped << "\n";

        } catch (const fs::filesystem_error& e) {
            std::cerr << "Erro ao percorrer diretórios: " << e.what() << "\n";
        }

        return headers;
    }

    bool FileWalker::isHeaderFile(const fs::path& path) {
        const std::string ext = toLower(path.extension().string());

        for (const auto& headerExt : HEADER_EXTENSIONS) {
            if (ext == headerExt) {
                return true;
            }
        }

        return false;
    }
}
//
// Created by bruno on 12/01/2026.
//

#ifndef CPP_SERIALIZER_FILEWALKER_H
#define CPP_SERIALIZER_FILEWALKER_H

#include <filesystem>
#include <vector>
#include <string>

namespace serializer {
    class FileWalker {
    public:
        static std::vector<std::filesystem::path> findHeaderFiles(const std::filesystem::path& projectPath) ;

    private:
        static bool isHeaderFile(const std::filesystem::path& path) ;

        std::vector<std::string> ignoreDirectories = {
            ".git", ".svn", ".vs", "build", "cmake-build-",
            "__pycache__", "node_modules"
        };
    };
}

#endif //CPP_SERIALIZER_FILEWALKER_H
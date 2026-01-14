//
// Created by bruno on 13/01/2026.
//

#include "include/Utils.h"

namespace serializer {
    std::string Utils::trim(const std::string& str) {
        auto start = str.find_first_not_of(" \t");
        if (start == std::string::npos) return "";

        auto end = str.find_last_not_of(" \t");
        return str.substr(start, end - start + 1);
    }

    std::string Utils::removeComments(std::string line) {
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        return trim(line);
    }

    std::pair<std::string, std::vector<std::string>> Utils::extractTemplateInfo(const std::string& typeName) {
        std::string baseType = typeName;
        std::vector<std::string> templateArgs;

        size_t templateStart = typeName.find('<');
        if (templateStart != std::string::npos && typeName.back() == '>') {
            baseType = typeName.substr(0, templateStart);

            std::string argsStr = typeName.substr(templateStart + 1,
                                                 typeName.length() - templateStart - 2);

            // Parsing simples de template args
            int bracketCount = 0;
            std::string currentArg;

            for (char c : argsStr) {
                if (c == '<') {
                    bracketCount++;
                    currentArg += c;
                } else if (c == '>') {
                    bracketCount--;
                    currentArg += c;
                } else if (c == ',' && bracketCount == 0) {
                    templateArgs.push_back(trim(currentArg));
                    currentArg.clear();
                } else {
                    currentArg += c;
                }
            }

            if (!currentArg.empty()) {
                templateArgs.push_back(trim(currentArg));
            }
        }

        return {baseType, templateArgs};
    }
}

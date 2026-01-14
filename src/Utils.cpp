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
}

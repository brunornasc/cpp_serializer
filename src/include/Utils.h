//
// Created by bruno on 13/01/2026.
//

#ifndef CPP_SERIALIZER_UTILS_H
#define CPP_SERIALIZER_UTILS_H

#include <string>
#include <vector>

namespace serializer {
    class Utils {
    public:
        static std::string trim(const std::string& str);
        static std::string removeComments(std::string line);
        static std::pair<std::string, std::vector<std::string>> extractTemplateInfo(const std::string& typeName);
    };
}


#endif //CPP_SERIALIZER_UTILS_H
//
// Created by bruno on 13/01/2026.
//

#ifndef CPP_SERIALIZER_UTILS_H
#define CPP_SERIALIZER_UTILS_H

#include <string>

namespace serializer {
    class Utils {
    public:
        static std::string trim(const std::string& str);
        static std::string removeComments(std::string line);
    };
}


#endif //CPP_SERIALIZER_UTILS_H
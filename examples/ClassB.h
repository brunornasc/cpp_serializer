//
// Created by bruno on 14/01/2026.
//

#ifndef CPP_SERIALIZER_CLASSB_H
#define CPP_SERIALIZER_CLASSB_H

#include <string>
#include <vector>

#include "../src/include/Macro.h"

SERIALIZABLE(ClassB)
class ClassB {
public:
    int idClasseB;

    TRANSIENT
    std::string naoDeveParsear;

    // Serialização
    // Serializa para JSON
    [[nodiscard]] nlohmann::json serialize() const;
    // Desserializa de JSON
    void deserialize(const nlohmann::json& json);
    // Cria instância a partir de JSON
    [[nodiscard]] static ClassB fromJson(const nlohmann::json& json);
    // Serialização genérica (Boost compatible)
    template<typename Archive>
    void serialize(Archive& ar) const;
    template<typename Archive>
    void deserialize(Archive& ar);
};
#endif //CPP_SERIALIZER_CLASSB_H
//
// Created by bruno on 13/01/2026.
//

#ifndef CPP_SERIALIZER_TEST_H
#define CPP_SERIALIZER_TEST_H
#include <string>
#include <vector>

#include "../src/include/Macro.h"

SERIALIZABLE(Usuario)
class Usuario {
public:
    int id;
    std::string nome;
    std::vector<std::string> emails;

    bool ativo;

    TRANSIENT
    std::string sessionToken;

    // Serialização
    // Serializa para JSON
    [[nodiscard]] nlohmann::json serialize() const;
    // Desserializa de JSON
    void deserialize(const nlohmann::json& json);
    // Cria instância a partir de JSON
    [[nodiscard]] static Usuario fromJson(const nlohmann::json& json);
    // Serialização genérica (Boost compatible)
    template<typename Archive>
    void serialize(Archive& ar) const;
    template<typename Archive>
    void deserialize(Archive& ar);
};
#endif //CPP_SERIALIZER_TEST_H


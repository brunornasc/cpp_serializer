// Arquivo gerado automaticamente por cpp-serializer-gen
// Não edite manualmente - será sobrescrito

#ifndef USUARIO_SERIALIZATION_IMPL_H
#define USUARIO_SERIALIZATION_IMPL_H

#include "test.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <map>
#include <optional>
#include <variant>

// Implementações de serialização para: Usuario

inline nlohmann::json Usuario::serialize() const {
    return nlohmann::json{
        {"id", id},
        {"nome", nome},
        {"emails", emails},
        {"ativo", ativo}
    };
}


inline void Usuario::deserialize(const nlohmann::json& json) {
    id = json["id"];
    nome = json["nome"];
    emails = json["emails"].get<std::vector<std::string>>();
    ativo = json["ativo"];
}


inline Usuario Usuario::fromJson(const nlohmann::json& json) {
    Usuario obj;
    obj.deserialize(json);
    return obj;
}


// Serialização genérica (compatível com Boost)
template<typename Archive>
inline void Usuario::serialize(Archive& ar) const {
    ar & id;
    ar & nome;
    ar & emails;
    ar & ativo;
}

template<typename Archive>
inline void Usuario::deserialize(Archive& ar) {
    ar & id;
    ar & nome;
    ar & emails;
    ar & ativo;
}

#endif // USUARIO_SERIALIZATION_IMPL_H

# CPP_SERIALIZER

# en-US

## A project to auto generate serialize/deserialize functions using the nlohmann::json library.

### PS: this project is a proof of concept and is NOT ready for production. keep that in mind.
### made with coding assistant's help (don't judge me I don't have much time)

## How this works

* 1 - Compile this project and generate the executable
* 2 - Execute passing the path of your sources
* 3 - It will search for all your header files where have the SERIALIZABLE(your_class_name)
* 4 - It will parse and generate a implementation file and link in your header, modifying to receive the new functions

### PS: only public variables will be parsed

## example

implementation

```CPP
#ifndef CPP_SERIALIZER_TEST_H
#define CPP_SERIALIZER_TEST_H
#include <string>
#include <vector>

#include "../src/include/Macro.h" // <-- the header you must include

SERIALIZABLE(Usuario) // <- the macro you must use
class Usuario {
public:
    int id;
    std::string nome;
    std::vector<std::string> emails;

    bool ativo;

    TRANSIENT // <- transient macro to ignore
    std::string sessionToken; // <- this will not be parsed
};
#endif //CPP_SERIALIZER_TEST_H
```

## usage

```CPP
#include "Usuario.h"
#include <iostream>

int main() {
    // Criar objeto
    Usuario user;
    user.id = 42;
    user.nome = "Bruno";
    user.emails = {"bruno@email.com", "contato@bruno.com"};
    
    // Serializar para JSON (retorna valor!)
    nlohmann::json json = user.serialize();
    std::cout << "Serializado:\n" << json.dump(2) << "\n\n";
    
    // Desserializar em novo objeto
    Usuario user2;
    user2.deserialize(json);
    std::cout << "Desserializado: " << user2.nome << " (ID: " << user2.id << ")\n";
    
    // Factory method
    auto user3 = Usuario::fromJson(json);
    std::cout << "Criado via factory: " << user3.nome << "\n";
    
    return 0;
}
```

# pt-BR

## Um projeto para gerar automaticamente funções de serialização/desserialização usando a biblioteca nlohmann::json.

### PS: este projeto é uma prova de conceito e NÃO está pronto para produção. Tenha isso em mente.

### Feito com a ajuda do assistente de codificação (não me julguem, eu não tenho muito tempo)

## Como funciona

* 1 - Compile este projeto e gere o executável
* 2 - Execute passando o caminho para seus arquivos de origem
* 3 - Ele buscará todos os seus arquivos de cabeçalho que contenham SERIALIZABLE(nome_da_sua_classe)
* 4 - Ele analisará e gerará um arquivo de implementação e vinculará seu cabeçalho, modificando-o para receber as novas funções

### PS: apenas variáveis públicas serão analisadas

## Exemplo

## Implementação

```CPP
#ifndef CPP_SERIALIZER_TEST_H
#define CPP_SERIALIZER_TEST_H
#include <string>
#include <vector>

#include "../src/include/Macro.h" // <-- você precisa incluir o header

SERIALIZABLE(Usuario) // <- a única macro obrigatória
class Usuario {
public:
    int id;
    std::string nome;
    std::vector<std::string> emails;

    bool ativo;

    TRANSIENT // <- transient macro vai ignorar a próxima variável
    std::string sessionToken; // <- não vai ser incluído
};
#endif //CPP_SERIALIZER_TEST_H
```

## uso

```CPP
#include "Usuario.h"
#include <iostream>

int main() {
    // Criar objeto
    Usuario user;
    user.id = 42;
    user.nome = "Bruno";
    user.emails = {"bruno@email.com", "contato@bruno.com"};
    
    // Serializar para JSON (retorna valor!)
    nlohmann::json json = user.serialize();
    std::cout << "Serializado:\n" << json.dump(2) << "\n\n";
    
    // Desserializar em novo objeto
    Usuario user2;
    user2.deserialize(json);
    std::cout << "Desserializado: " << user2.nome << " (ID: " << user2.id << ")\n";
    
    // Factory method
    auto user3 = Usuario::fromJson(json);
    std::cout << "Criado via factory: " << user3.nome << "\n";
    
    return 0;
}
```

#pragma once
#include <string>
#include <unordered_map>
#include <functional>

class RTTI;

class Factory {
private:
    // Le registre : Map < "NomDeLaClasse", PointeurVersFonctionDeCreation >
    static std::unordered_map<std::string, std::function<RTTI* ()>>& GetRegistry() {
        static std::unordered_map<std::string, std::function<RTTI* ()>> registry;
        return registry;
    }

public:
    // Enregistre une nouvelle classe dans le dictionnaire
    static void Register(const std::string& className, std::function<RTTI* ()> creatorFunc) {
        GetRegistry()[className] = creatorFunc;
    }

    // "reconstruire un object à partir d'un clé"
    static RTTI* Create(const std::string& className) {
        auto& reg = GetRegistry();
        if (reg.find(className) != reg.end()) {
            return reg[className](); // Appelle la fonction de création
        }
        return nullptr; // La classe n'existe pas
    }
};
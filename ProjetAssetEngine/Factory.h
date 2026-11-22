#pragma once
#include <string>
#include <unordered_map>
#include <functional>

class RTTI;

class Factory {
private:
    [[nodiscard]] static inline std::unordered_map<std::string, std::function<RTTI* ()>>& GetRegistry() {
        static std::unordered_map<std::string, std::function<RTTI* ()>> registry;
        return registry;
    }

public:
    // Enregistre une nouvelle classe dans le dictionnaire
    [[nodiscard]] static inline void Register(const std::string& className, std::function<RTTI* ()> creatorFunc) {
        GetRegistry()[className] = creatorFunc;
    }

    // "reconstruire un object à partir d'un clé"
    [[nodiscard]] static inline RTTI* Create(const std::string& className) {
        auto& reg = GetRegistry();
        if (reg.find(className) != reg.end()) {
            return reg[className](); // Appelle la fonction de création
        }
        return nullptr; // La classe n'existe pas
    }
};
#pragma once

#include "RTTI.h"
#include "Archive.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>

// CLASSE FAITE POUR SIMULER UN ASSET (UTILISER POUR IMGUI) !

class Pawn : public RTTI {
    DECLARE_RTTI(Pawn)

public:
    int health = 100;
    float speed = 10.0f;

    std::vector<int> inventory = { 10, 20, 30 };

    std::vector<std::vector<int>> grid = {
        {1, 0, 1},
        {0, 2, 0}
    };

    std::unordered_map<std::string, int> stats = {
        {"Strength", 15},
        {"Agility", 22}
    };

    RTTI* equippedWeapon = nullptr;

    std::string sourceFilePath = "";

    Pawn() = default;
    ~Pawn() {
        if (equippedWeapon) delete equippedWeapon;
    }

    void Process(Archive& ar) override {
        ar.Process("Health", health);
        ar.Process("Speed", speed);
        ar.Process("Inventory", inventory);
        ar.Process("Grid", grid);
        ar.Process("Stats", stats);
        ar.Process("EquippedWeapon", equippedWeapon);

    }

    void Print() const {
        std::cout << "  -> Pawn [Health: " << health << ", Speed: " << speed << "]\n";

        if (!sourceFilePath.empty()) {
            std::cout << "     Fichier source : " << sourceFilePath << "\n";
        }

        std::cout << "     Inventaire (" << inventory.size() << " items): ";
        for (int id : inventory) std::cout << id << " ";
        std::cout << "\n";

        std::cout << "     Grille 2D (Taille: " << grid.size() << "x" << grid[0].size() << ")\n";

        std::cout << "     Stats:\n";
        for (auto& pair : stats) {
            std::cout << "       - " << pair.first << " : " << pair.second << "\n";
        }

        if (equippedWeapon != nullptr) {
            std::cout << "     Arme equipee (Polymorphe) de type : " << equippedWeapon->GetClassName() << "\n";
        }
        else {
            std::cout << "     Aucune arme equipee.\n";
        }
    }
};

class Weapon : public RTTI {
    DECLARE_RTTI(Weapon)

public:
    int damage = 50;

    Weapon() = default;

    void Process(Archive& ar) override {
        ar.Process("Damage", damage);
    }
};
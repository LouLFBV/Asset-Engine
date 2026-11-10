#pragma once
#include "RTTI.h"
#include "Archive.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>

class Pawn : public RTTI {
    DECLARE_RTTI(Pawn)

public:
    int health = 100;
    float speed = 10.0f;

    // 1. Tableaux simples
    std::vector<int> inventory = { 10, 20, 30 };

    // 2. Tableaux imbriqués (Grille 2D)
    std::vector<std::vector<int>> grid = {
        {1, 0, 1},
        {0, 2, 0}
    };

    // 3. Maps
    std::unordered_map<std::string, int> stats = {
        {"Strength", 15},
        {"Agility", 22}
    };

    // 4. Type polymorphe (Pointeur vers une classe dérivée de RTTI)
    RTTI* equippedWeapon = nullptr;

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
        ar.Process("EquippedWeapon", equippedWeapon); // Gère le polymorphisme !
    }

    void Print() const {
        std::cout << "  -> Pawn [Health: " << health << ", Speed: " << speed << "]\n";

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
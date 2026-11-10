#pragma once

#include <string>

#include "Factory.h"

class Archive; 

class RTTI {
public:
    virtual ~RTTI() = default; 

    virtual const std::string& GetClassName() const = 0;

    // Toutes les classes du jeu pourront se sérialiser
    virtual void Process(Archive& ar) = 0;
};



// Elle génère automatiquement la méthode GetClassName et un constructeur statique.
// Le "#ClassName" transforme le nom que tu tapes en std::string (ex: Pawn devient "Pawn").
#define DECLARE_RTTI(ClassName) \
public: \
    const std::string& GetClassName() const override { \
        static const std::string name = #ClassName; \
        return name; \
    } \
    static RTTI* CreateInstance() { \
        return new ClassName(); \
    }

// Elle exécute du code AVANT le lancement du main() (avant le runtime).
// Elle crée une petite structure globale dont le constructeur s'inscrit dans la Factory.
#define REGISTER_RTTI(ClassName) \
    namespace { \
        struct ClassName##_Register { \
            ClassName##_Register() { \
                Factory::Register(#ClassName, &ClassName::CreateInstance); \
            } \
        }; \
        static ClassName##_Register global_##ClassName##_Register; \
    }
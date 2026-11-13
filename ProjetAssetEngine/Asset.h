#pragma once
#include <string>

#include "Guid.h"

// Déclaration anticipée de la classe de base dont toutes tes classes vont hériter
class RTTI;

struct Asset {
	uint32_t checksum = 0; // Pour vérifier l'intégrité du fichier
    Guid uiid;
    std::string path;

    // "type RTTI (classe)" : la string qui permet de retrouver la classe dans le registre
    std::string type;

    // "objet s'il est load (un ptr)" : pointeur vers la classe de base
    RTTI* objet = nullptr;
};
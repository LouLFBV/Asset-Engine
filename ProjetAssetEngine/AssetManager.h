#pragma once
#include <unordered_map>
#include <chrono>
#include "Asset.h"
#include "Factory.h"
#include "JsonArchive.h"
#include "RTTI.h"
#include "JobSystem.h"

class AssetManager {
private:
    JobSystem m_assetJobSystem;
    std::unordered_map<std::string, Asset*> m_assetTree;

    // Le conteneur secret pour ne pas faire lagger le jeu !
    // Il associe l'UUID de l'asset à sa tâche de chargement en arrière-plan.
    std::unordered_map<std::string, Job> m_loadingTasks;

public:
    void LoadSync(Asset* asset);

    //ASYNCHRONE (Ne bloque pas le jeu)
    void LoadAsync(Asset* asset);

    // MISE A JOUR (A appeler à chaque frame dans ton jeu)
    void Update();
};
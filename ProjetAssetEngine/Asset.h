
#pragma once

#include <string>
#include <cstdint>
#include <mutex>

#include "Guid.h"

class RTTI;

struct Asset
{
    uint32_t checksum = 0;

    Guid uiid;

    std::string path;
    std::string type;
        
    // Objet charge en memoire.
    RTTI* objet = nullptr;

    // Synchronisation entre le thread principal et les workers.
    mutable std::mutex mutex;

    // Consultation securisee de l'objet.
    RTTI* GetObject() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return objet;
    }

    bool IsLoaded() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return objet != nullptr;
    }
};

#include "Log.h"

int main() {
    // 1. Initialisation du Log
    Log::Init("engine_debug.log");

    LOG_INFO("Moteur demarre avec succes.");
    LOG_WARN("Attention, la ressource texture_01 est introuvable, utilisation du placeholder.");
    LOG_ERROR("Impossible d'ouvrir le fichier d'archive !");

    // 2. Fermeture propre
    Log::Shutdown();
    return 0;
}
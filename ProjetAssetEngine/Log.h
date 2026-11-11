#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <mutex>

enum class LogLevel {
    Info,
    Warning,
    Error
};

class Log {
private:
    static std::ofstream s_LogFile;
    static std::mutex s_Mutex; // Pour éviter les conflits si plusieurs threads loguent en même temps (JobSystem !)

public:
    // Initialise le fichier de log au démarrage du moteur
    static void Init(const std::string& filepath = "engine.log");

    // Ferme proprement le fichier à la fin
    static void Shutdown();

    // Méthode centrale d'écriture
    static void Print(LogLevel level, const std::string& message);
};

// --- MACROS DE CONFORT ---
// Ça évite d'écrire Log::Print à chaque fois et ça rend le code plus propre
#define LOG_INFO(msg)    Log::Print(LogLevel::Info, msg)
#define LOG_WARN(msg)    Log::Print(LogLevel::Warning, msg)
#define LOG_ERROR(msg)   Log::Print(LogLevel::Error, msg)
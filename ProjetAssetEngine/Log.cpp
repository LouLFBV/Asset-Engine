#include "Log.h"
#include <chrono>
#include <iomanip>

// Initialisation des variables statiques
std::ofstream Log::s_LogFile;
std::mutex Log::s_Mutex; // Note: utilise std::mutex s_Mutex;

void Log::Init(const std::string& filepath) {
    // Ouvre le fichier en écriture (écrase l'ancien log à chaque lancement)
    s_LogFile.open(filepath, std::ios::out);
}

void Log::Shutdown() {
    if (s_LogFile.is_open()) {
        s_LogFile.close();
    }
}

void Log::Print(LogLevel level, const std::string& message) {
    // Récupération de l'heure actuelle de manière sécurisée et moderne (C++20)
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm timeInfo;
    // localtime_s est sécurisée et fonctionne nativement sur Windows (MSVC)
    // Attention à l'ordre des arguments sous Windows : (struct tm* _Tm, const time_t* _Time)
    localtime_s(&timeInfo, &in_time_t);

    std::string levelStr;
    std::string colorCode;

    switch (level) {
    case LogLevel::Info:
        levelStr = "INFO";
        colorCode = "\033[32m"; // Vert
        break;
    case LogLevel::Warning:
        levelStr = "WARNING";
        colorCode = "\033[33m"; // Jaune
        break;
    case LogLevel::Error:
        levelStr = "ERROR";
        colorCode = "\033[31m"; // Rouge
        break;
    }

    std::string resetColor = "\033[0m";

    char timeBuffer[32];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", &timeInfo);

    std::string formattedMessage = std::string("[") + timeBuffer + "] [" + levelStr + "] " + message;

    {
        // std::lock_guard<std::mutex> lock(s_Mutex);

        std::cout << colorCode << formattedMessage << resetColor << std::endl;

        if (s_LogFile.is_open()) {
            s_LogFile << formattedMessage << std::endl;
        }
    }
}
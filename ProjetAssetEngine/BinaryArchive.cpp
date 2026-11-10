#include "BinaryArchive.h"

BinaryArchive::BinaryArchive(const std::string& filepath, ArchiveDirection dir)
    : Archive(dir)
{
    // Définition des modes d'ouverture de fichier
    std::ios_base::openmode mode = std::ios::binary;

    if (IsReading()) {
        mode |= std::ios::in;
    }
    else {
        mode |= std::ios::out | std::ios::trunc; // trunc efface le fichier existant
    }

    m_stream.open(filepath, mode);

    if (!m_stream.is_open()) {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier " << filepath << std::endl;
    }
}

BinaryArchive::~BinaryArchive() {
    if (m_stream.is_open()) {
        m_stream.close();
    }
}


void BinaryArchive::Process(const char* Key, int& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, float& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, double& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, bool& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, char& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, uint16_t& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, uint32_t& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, int16_t& Data) {
    if (IsReading()) m_stream.read(reinterpret_cast<char*>(&Data), sizeof(Data));
    else             m_stream.write(reinterpret_cast<const char*>(&Data), sizeof(Data));
}

void BinaryArchive::Process(const char* Key, std::string& Data) {
    if (IsReading()) {
        // 1. Lire la taille de la chaine
        uint32_t length = 0;
        m_stream.read(reinterpret_cast<char*>(&length), sizeof(length));

        // 2. Préparer la chaine et lire les caractères
        Data.resize(length);
        if (length > 0) {
            m_stream.read(&Data[0], length); // En C++11 et +, c'est parfaitement valide
        }
    }
    else {
        // 1. Écrire la taille de la chaine
        uint32_t length = static_cast<uint32_t>(Data.size());
        m_stream.write(reinterpret_cast<const char*>(&length), sizeof(length));

        // 2. Écrire les caractères
        if (length > 0) {
            m_stream.write(Data.c_str(), length);
        }
    }
}
#include "JsonArchive.h"
#include <fstream>
#include <iostream>

JsonArchive::JsonArchive(const std::string& filepath, ArchiveDirection dir) : Archive(dir), m_filepath(filepath)
{
    if (IsReading()) {
        std::ifstream file(m_filepath);
        if (file.is_open()) {
            // Magie de nlohmann : parse tout le fichier texte vers l'objet JSON
            file >> m_jsonData;
            file.close();
        }
        else {
            std::cerr << "Erreur: Impossible de lire le fichier JSON " << filepath << std::endl;
        }
    }
    // En écriture, on ne fait rien dans le constructeur. L'objet JSON est vide.
}

JsonArchive::~JsonArchive() {
    if (IsWriting()) {
        std::ofstream file(m_filepath);
        if (file.is_open()) {
            // Le "4" permet une belle indentation (pretty-print) avec 4 espaces.
            file << m_jsonData.dump(4);
            file.close();
        }
    }
}

void JsonArchive::Process(const char* Key, int& Data) {
    if (!Key) return; // Sécurité : en JSON, il faut obligatoirement une clé !

    if (IsReading()) {
        // On vérifie que la clé existe avant d'essayer de lire
        if (m_jsonData.contains(Key)) {
            Data = m_jsonData[Key].get<int>();
        }
    }
    else {
        // En écriture, c'est aussi simple que ça
        m_jsonData[Key] = Data;
    }
}

void JsonArchive::Process(const char* Key, std::string& Data) {
    if (!Key) return;

    if (IsReading()) {
        if (m_jsonData.contains(Key)) {
            Data = m_jsonData[Key].get<std::string>();
        }
    }
    else {
        m_jsonData[Key] = Data;
    }
}

void JsonArchive::Process(const char* Key, uint16_t& Data)
{
    if (!Key) return;

	if (IsReading()) {
		if (m_jsonData.contains(Key)) {
			Data = m_jsonData[Key].get<uint16_t>();
		}
	}
	else {
		m_jsonData[Key] = Data;
	}
}

void JsonArchive::Process(const char* Key, uint32_t & Data)
{
	if (!Key) return;

	if (IsReading()) {
		if (m_jsonData.contains(Key)) {
			Data = m_jsonData[Key].get<uint32_t>();
		}
	}
	else {
		m_jsonData[Key] = Data;
	}
}

void JsonArchive::Process(const char* Key, int16_t & Data)
{
    if (!Key)  return;

	if (IsReading()) {
		if (m_jsonData.contains(Key)) {
			Data = m_jsonData[Key].get<int16_t>();
		}
	}
	else {
		m_jsonData[Key] = Data;
	}   
}

void JsonArchive::Process(const char* Key, float& Data) {
    if (!Key) return;

    if (IsReading()) {
        if (m_jsonData.contains(Key)) {
            Data = m_jsonData[Key].get<float>();
        }
    }
    else {
        m_jsonData[Key] = Data;
    }
}

void JsonArchive::Process(const char* Key, double& Data)
{
    if (!Key) return;

    if (IsReading()) {
        if (m_jsonData.contains(Key)) {
            Data = m_jsonData[Key].get<double>();
        }
    }
    else {
        m_jsonData[Key] = Data;
    }
}

void JsonArchive::Process(const char* Key, bool& Data)
{
    if (!Key) return;

    if (IsReading()) {
        if (m_jsonData.contains(Key)) {
            Data = m_jsonData[Key].get<bool>();
        }
    }
    else {
        m_jsonData[Key] = Data;
    }
}

void JsonArchive::Process(const char* Key, char& Data)
{
    if (!Key) return;

    if (IsReading()) {
        if (m_jsonData.contains(Key)) {
            Data = m_jsonData[Key].get<char>();
        }
    }
    else {
        m_jsonData[Key] = Data;
    }
}

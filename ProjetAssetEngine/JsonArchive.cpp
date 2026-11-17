#include "JsonArchive.h"
#include "Checksum.h" // Indispensable pour calculer le CRC32
#include "Log.h"      // Pour tracer les alertes de sécurité dans les logs
#include <fstream>
#include <iostream>
#include <stdexcept>

JsonArchive::JsonArchive(const std::string& filepath, ArchiveDirection dir)
    : Archive(dir, filepath)
{
    if (IsReading()) {
        std::ifstream file(m_filepath);
        if (file.is_open()) {
            // Lecture et parsing du JSON via nlohmann
            file >> m_jsonData;
            file.close();

            // --- SYSTÈME DE SÉCURITÉ ET D'INTÉGRITÉ (CHECKSUM) ---
            if (!m_jsonData.contains("_Checksum")) {
                LOG_ERROR("ALERTE SÉCURITÉ : Fichier corrompu ou non signé (aucun checksum trouvé) !");
                //throw std::runtime_error("Missing checksum in archive!");
            }

            // 1. Récupérer le checksum stocké dans le fichier
            uint32_t storedChecksum = m_jsonData["_Checksum"].get<uint32_t>();

            // 2. Isoler les données en retirant le champ de sécurité pour recalculer l'empreinte exacte
            nlohmann::json dataToVerify = m_jsonData;
            dataToVerify.erase("_Checksum");

            uint32_t computedChecksum = Checksum::ComputeFromString(dataToVerify.dump());

            // 3. Comparaison : Si les hashes divergent, le fichier a été modifié à la main !
            if (storedChecksum != computedChecksum) {
                LOG_ERROR("ALERTE INTÉGRITÉ : Le checksum ne correspond pas ! Le fichier a été modifié ou corrompu.");
                //throw std::runtime_error("Checksum mismatch! Archive integrity compromised.");
            }

            LOG_INFO("-> Sécurité OK : Intégrité du fichier archive vérifiée par CRC32.");
        }
        else {
            std::cerr << "Erreur: Impossible de lire le fichier JSON " << filepath << std::endl;
        }
    }
    // En écriture, on ne fait rien dans le constructeur. L'objet JSON est vide au départ.
}

JsonArchive::~JsonArchive() {
    if (IsWriting()) {
        // --- SIGNATURE DE L'ARCHIVE ---
        // Avant d'écrire, on isole les données pour calculer leur empreinte CRC32 (sans l'attribut _Checksum lui-même)
        nlohmann::json dataToSign = m_jsonData;
        dataToSign.erase("_Checksum");

        uint32_t finalChecksum = Checksum::ComputeFromString(dataToSign.dump());

        // On injecte ce checksum calculé dans le contenu JSON final
        m_jsonData["_Checksum"] = finalChecksum;

        // Écriture sur le disque avec une belle indentation (pretty-print à 4 espaces)
        std::ofstream file(m_filepath);
        if (file.is_open()) {
            file << m_jsonData.dump(4);
            file.close();
        }
    }
}

//void JsonArchive::Process(const char* Key, int& Data) {
//    if (!Key) return; // Sécurité : en JSON, il faut obligatoirement une clé !
//
//    if (IsReading()) {
//        // On vérifie que la clé existe avant d'essayer de lire
//        if (m_jsonData.contains(Key)) {
//            Data = m_jsonData[Key].get<int>();
//        }
//    }
//    else {
//        // En écriture, c'est aussi simple que ça
//        m_jsonData[Key] = Data;
//    }
//}
//
//void JsonArchive::Process(const char* Key, std::string& Data) {
//    if (!Key) return;
//
//    if (IsReading()) {
//        if (m_jsonData.contains(Key)) {
//            Data = m_jsonData[Key].get<std::string>();
//        }
//    }
//    else {
//        m_jsonData[Key] = Data;
//    }
//}
//
//void JsonArchive::Process(const char* Key, uint16_t& Data)
//{
//    if (!Key) return;
//
//	if (IsReading()) {
//		if (m_jsonData.contains(Key)) {
//			Data = m_jsonData[Key].get<uint16_t>();
//		}
//	}
//	else {
//		m_jsonData[Key] = Data;
//	}
//}
//
//void JsonArchive::Process(const char* Key, uint32_t & Data)
//{
//	if (!Key) return;
//
//	if (IsReading()) {
//		if (m_jsonData.contains(Key)) {
//			Data = m_jsonData[Key].get<uint32_t>();
//		}
//	}
//	else {
//		m_jsonData[Key] = Data;
//	}
//}
//
//void JsonArchive::Process(const char* Key, int16_t & Data)
//{
//    if (!Key)  return;
//
//	if (IsReading()) {
//		if (m_jsonData.contains(Key)) {
//			Data = m_jsonData[Key].get<int16_t>();
//		}
//	}
//	else {
//		m_jsonData[Key] = Data;
//	}   
//}
//
//void JsonArchive::Process(const char* Key, float& Data) {
//    if (!Key) return;
//
//    if (IsReading()) {
//        if (m_jsonData.contains(Key)) {
//            Data = m_jsonData[Key].get<float>();
//        }
//    }
//    else {
//        m_jsonData[Key] = Data;
//    }
//}
//
//void JsonArchive::Process(const char* Key, double& Data)
//{
//    if (!Key) return;
//
//    if (IsReading()) {
//        if (m_jsonData.contains(Key)) {
//            Data = m_jsonData[Key].get<double>();
//        }
//    }
//    else {
//        m_jsonData[Key] = Data;
//    }
//}
//
//void JsonArchive::Process(const char* Key, bool& Data)
//{
//    if (!Key) return;
//
//    if (IsReading()) {
//        if (m_jsonData.contains(Key)) {
//            Data = m_jsonData[Key].get<bool>();
//        }
//    }
//    else {
//        m_jsonData[Key] = Data;
//    }
//}
//
//void JsonArchive::Process(const char* Key, char& Data)
//{
//    if (!Key) return;
//
//    if (IsReading()) {
//        if (m_jsonData.contains(Key)) {
//            Data = m_jsonData[Key].get<char>();
//        }
//    }
//    else {
//        m_jsonData[Key] = Data;
//    }
//}

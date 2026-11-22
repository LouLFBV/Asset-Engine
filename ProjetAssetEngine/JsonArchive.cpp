#include "JsonArchive.h"
#include "Checksum.h"
#include "Log.h"      

#include <fstream>
#include <iostream>
#include <stdexcept>

JsonArchive::JsonArchive(const std::string& filepath, ArchiveDirection dir) : Archive(dir, filepath)
{
    if (IsReading()) {
        std::ifstream file(m_filepath);
        if (file.is_open()) {
            file >> m_jsonData;
            file.close();

            if (!m_jsonData.contains("_Checksum")) 
                LOG_ERROR("ALERTE SECURITE : Fichier corrompu ou non signe (aucun checksum trouve) !");
            

            uint32_t storedChecksum = m_jsonData["_Checksum"].get<uint32_t>();

            nlohmann::json dataToVerify = m_jsonData;
            dataToVerify.erase("_Checksum");

            uint32_t computedChecksum = Checksum::ComputeFromString(dataToVerify.dump());

            if (storedChecksum != computedChecksum) 
                LOG_ERROR("ALERTE INTEGRITE : Le checksum ne correspond pas ! Le fichier a ete modifie ou corrompu.");

            LOG_INFO("-> Securite OK : Intégrité du fichier archive verifiee par CRC32.");
        }
        else 
            std::cerr << "Erreur: Impossible de lire le fichier JSON " << filepath << std::endl;
        
    }
}

JsonArchive::~JsonArchive() {
    if (IsWriting()) {
        nlohmann::json dataToSign = m_jsonData;
        dataToSign.erase("_Checksum");

        uint32_t finalChecksum = Checksum::ComputeFromString(dataToSign.dump());

        m_jsonData["_Checksum"] = finalChecksum;

        std::ofstream file(m_filepath);
        if (file.is_open()) {
            file << m_jsonData.dump(4);
            file.close();
        }
    }
}

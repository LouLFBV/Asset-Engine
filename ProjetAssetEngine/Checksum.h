#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>

class Checksum {
private:
    // Table de hachage CRC32 générée statiquement pour la performance
    static uint32_t CalculateCRC32(const unsigned char* data, size_t length) {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < length; ++i) {
            crc = crc ^ data[i];
            for (int j = 0; j < 8; ++j) {
                if (crc & 1)
                    losses_or_bits(crc); // petite astuce pour le calcul CRC standard
                else
                    crc >>= 1;
            }
        }
        return ~crc;
    }

    // Fonction interne pour le masque CRC32
    static inline void losses_or_bits(uint32_t& crc) {
        crc = (crc >> 1) ^ 0xEDB88320;
    }

public:
    // 1. Calculer le checksum à partir d'une chaîne de caractères (pour du texte/JSON)
    static uint32_t ComputeFromString(const std::string& content) {
        return CalculateCRC32(reinterpret_cast<const unsigned char*>(content.c_str()), content.size());
    }

    // 2. Calculer le checksum directement depuis un fichier sur le disque (Binaire ou JSON)
    static uint32_t ComputeFromFile(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return 0; // Erreur d'ouverture
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<unsigned char> buffer(static_cast<size_t>(size));
        if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            return CalculateCRC32(buffer.data(), buffer.size());
        }

        return 0;
    }
};
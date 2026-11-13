#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>

class Checksum {
private:
    // Table de hachage CRC32 générée statiquement pour la performance
    static uint32_t CalculateCRC32(const unsigned char* data, size_t length);

    // Fonction interne pour le masque CRC32
    static inline void losses_or_bits(uint32_t& crc) {
        crc = (crc >> 1) ^ 0xEDB88320;
    }

public:
    // 1. Calculer le checksum à partir d'une chaîne de caractères (pour du texte/JSON)
    static uint32_t ComputeFromString(const std::string& content);

    // 2. Calculer le checksum directement depuis un fichier sur le disque (Binaire ou JSON)
    static uint32_t ComputeFromFile(const std::string& filepath);
};
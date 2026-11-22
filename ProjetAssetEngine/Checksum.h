#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>

class Checksum {
private:

    static inline void LossesOrBits(uint32_t& crc) noexcept {
        crc = (crc >> 1) ^ 0xEDB88320;
    }

public:
    static uint32_t ComputeFromString(const std::string& content);

    static uint32_t ComputeFromFile(const std::string& filepath);

    static uint32_t CalculateCRC32(const unsigned char* data, size_t length) noexcept;
};
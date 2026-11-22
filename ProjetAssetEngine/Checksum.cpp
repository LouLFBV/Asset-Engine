#include "Checksum.h"

#include <limits>

uint32_t Checksum::CalculateCRC32(const unsigned char* data, size_t length) noexcept
{
    uint32_t crc = std::numeric_limits<uint32_t>::max(); 
    for (size_t i = 0; i < length; ++i) 
    { 
        crc = crc ^ data[i]; 
        for (int j = 0; j < 8; ++j) 
        {
            if (crc & 1) 
                LossesOrBits(crc); 
            else
                crc >>= 1; 
        }
    }
    return ~crc; 
}

uint32_t Checksum::ComputeFromString(const std::string& content)
{
    return CalculateCRC32(reinterpret_cast<const unsigned char*>(content.c_str()), content.size());
}

uint32_t Checksum::ComputeFromFile(const std::string& filepath) 
{
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) 
        return 0; 

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(static_cast<size_t>(size)); 
    if (file.read(reinterpret_cast<char*>(buffer.data()), size))
        return CalculateCRC32(buffer.data(), buffer.size());

    return 0;
}
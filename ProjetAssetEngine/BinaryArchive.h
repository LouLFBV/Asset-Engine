#pragma once

#include <iostream>
#include <fstream>

#include "Archive.h"

class BinaryArchive : public Archive {
private:
	std::fstream m_stream;

public:
    BinaryArchive(const std::string& filepath, ArchiveDirection dir);

    ~BinaryArchive() override;

    // Surcharges des méthodes de la classe parent
    void Process(const char* Key, int& Data) override;
    void Process(const char* Key, float& Data) override;
    void Process(const char* Key, double& Data) override;
    void Process(const char* Key, bool& Data) override;
    void Process(const char* Key, char& Data) override;
    void Process(const char* Key, std::string& Data) override;

    void Process(const char* Key, uint16_t& Data) override { Process<uint16_t>(Key, Data); }
    void Process(const char* Key, uint32_t& Data) override { Process<uint32_t>(Key, Data); }
    void Process(const char* Key, int16_t& Data) override { Process<int16_t>(Key, Data); }

    template <typename T>
        requires std::is_arithmetic_v<T>
    void Process(const char* key, T& data) { 
        if (IsReading()) {
            m_stream.read(reinterpret_cast<char*>(&data), sizeof(T));
        }
        else {
            m_stream.write(reinterpret_cast<const char*>(&data), sizeof(T));
        }
    }

    bool IsValid() const { return m_stream.is_open() && m_stream.good(); }
};
#pragma once

#include <fstream>

#include "Archive.h"

class BinaryArchive : public Archive {
private:
	std::fstream m_stream;

public:
    BinaryArchive(const std::string& filepath, ArchiveDirection dir);

    ~BinaryArchive() override;

    void Process(const char* Key, int& Data) override { Process<int>(Key, Data); }
    void Process(const char* Key, float& Data) override { Process<float>(Key, Data); }
    void Process(const char* Key, double& Data) override { Process<double>(Key, Data); }
    void Process(const char* Key, bool& Data) override { Process<bool>(Key, Data); }
    void Process(const char* Key, char& Data) override { Process<char>(Key, Data); }

    void Process(const char* Key, std::string& Data) override;

    void Process(const char* Key, uint16_t& Data) override { Process<uint16_t>(Key, Data); }
    void Process(const char* Key, uint32_t& Data) override { Process<uint32_t>(Key, Data); }
    void Process(const char* Key, int16_t& Data) override { Process<int16_t>(Key, Data); }

    template <typename T>
    void Process(const char* key, T& data) { 
        if (IsReading()) {
            m_stream.read(reinterpret_cast<char*>(&data), sizeof(T));
        }
        else {
            m_stream.write(reinterpret_cast<const char*>(&data), sizeof(T));
        }
    }

    [[nodiscard]] inline bool IsValid() const noexcept { return m_stream.is_open() && m_stream.good(); }
};
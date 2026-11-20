#pragma once

#include "Archive.h"
#include "json.h"
#include <string>

// Alias pour simplifier l'écriture
using json = nlohmann::json;

class JsonArchive : public Archive {
private:
    json m_jsonData;        // L'objet JSON gardé en mémoire tampon (buffer)

public:
    JsonArchive(const std::string& filepath, ArchiveDirection dir);
    ~JsonArchive() override;

    template <typename T>
    void Process(const char* Key, T& Data) {
        if (!Key) return; 

        if (IsReading()) {
            if (m_jsonData.contains(Key)) {
                Data = m_jsonData[Key].get<T>();
            }
        }
        else {
            m_jsonData[Key] = Data;
        }
    }

    void Process(const char* Key, int& Data) override { Process<int>(Key, Data); }
    void Process(const char* Key, float& Data) override { Process<float>(Key, Data); }
    void Process(const char* Key, double& Data) override { Process<double>(Key, Data); }
    void Process(const char* Key, bool& Data) override { Process<bool>(Key, Data); }
    void Process(const char* Key, char& Data) override { Process<char>(Key, Data); }
    void Process(const char* Key, std::string& Data) override { Process<std::string>(Key, Data); }

    void Process(const char* Key, uint16_t& Data) override { Process<uint16_t>(Key, Data); }
    void Process(const char* Key, uint32_t& Data) override { Process<uint32_t>(Key, Data); }
    void Process(const char* Key, int16_t& Data) override { Process<int16_t>(Key, Data); }

    //void Process(const char* Key, int& Data) override;
    //void Process(const char* Key, float& Data) override;
    //void Process(const char* Key, double& Data) override;
    //void Process(const char* Key, bool& Data) override;
    //void Process(const char* Key, char& Data) override;
    //void Process(const char* Key, std::string& Data) override;

    //void Process(const char* Key, uint16_t& Data) override;
    //void Process(const char* Key, uint32_t& Data) override;
    //void Process(const char* Key, int16_t& Data) override;
};
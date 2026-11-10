#pragma once

#include "Archive.h"
#include "json.h"
#include <string>

// Alias pour simplifier l'écriture
using json = nlohmann::json;

class JsonArchive : public Archive {
private:
    std::string m_filepath;
    json m_jsonData;        // L'objet JSON gardé en mémoire tampon (buffer)

public:
    JsonArchive(const std::string& filepath, ArchiveDirection dir);
    ~JsonArchive() override;

    void Process(const char* Key, int& Data) override;
    void Process(const char* Key, float& Data) override;
    void Process(const char* Key, double& Data) override;
    void Process(const char* Key, bool& Data) override;
    void Process(const char* Key, char& Data) override;
    void Process(const char* Key, std::string& Data) override;

    void Process(const char* Key, uint16_t& Data) override;
    void Process(const char* Key, uint32_t& Data) override;
    void Process(const char* Key, int16_t& Data) override;
};
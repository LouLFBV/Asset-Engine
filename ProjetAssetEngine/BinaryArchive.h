#pragma once

#include <iostream>
#include <fstream>

#include "Archive.h"

class BinaryArchive : Archive {
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

    void Process(const char* Key, uint16_t& Data) override;
    void Process(const char* Key, uint32_t& Data) override;
    void Process(const char* Key, int16_t& Data) override;

    bool IsValid() const { return m_stream.is_open() && m_stream.good(); }
};
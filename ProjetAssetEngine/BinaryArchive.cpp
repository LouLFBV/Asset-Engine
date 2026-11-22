#include "BinaryArchive.h"
#include "Checksum.h" 
#include "Log.h"      
#include <vector>
#include <stdexcept>
#include <iostream>

BinaryArchive::BinaryArchive(const std::string& filepath, ArchiveDirection dir) : Archive(dir, filepath)
{
    std::ios_base::openmode mode = std::ios::binary;

    if (IsReading())
        mode |= std::ios::in;
    else 
        mode |= std::ios::out | std::ios::trunc;

    m_stream.open(filepath, mode);

    if (!m_stream.is_open()) 
        std::cerr << "Erreur: Impossible d'ouvrir le fichier " << filepath << std::endl;
}

BinaryArchive::~BinaryArchive() {
    if (m_stream.is_open()) 
        m_stream.close();    
}



void BinaryArchive::Process(const char* Key, std::string& Data) {
    if (IsReading())
    {
        uint32_t length = 0;
        m_stream.read(reinterpret_cast<char*>(&length), sizeof(length));
        Data.resize(length);
        if (length > 0) 
            m_stream.read(&Data[0], length);
    }
    else 
    {
        uint32_t length = static_cast<uint32_t>(Data.size());
        m_stream.write(reinterpret_cast<const char*>(&length), sizeof(length));

        if (length > 0) 
            m_stream.write(Data.c_str(), length);
        
    }
}
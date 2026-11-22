#pragma once

#include <string>
#include <cstdint> 
#include <vector>       
#include <unordered_map>

enum class ArchiveDirection { Read, Write };

class Archive {
protected:
    ArchiveDirection m_direction;
    std::string m_filepath;

public:
    explicit Archive(ArchiveDirection dir, std::string filepath) : m_direction(dir), m_filepath(filepath) {};
    virtual ~Archive() = default;

    [[nodiscard]] inline bool IsReading() const noexcept{ return m_direction == ArchiveDirection::Read; }
    [[nodiscard]] inline bool IsWriting() const noexcept { return m_direction == ArchiveDirection::Write; }

    virtual void Process(const char* Key, int& Data) = 0;
    virtual void Process(const char* Key, float& Data) = 0;
    virtual void Process(const char* Key, std::string& Data) = 0;
    virtual void Process(const char* Key, double& Data) = 0;
    virtual void Process(const char* Key, bool& Data) = 0;
    virtual void Process(const char* Key, char& Data) = 0;

    virtual void Process(const char* Key, uint16_t& Data) = 0;
    virtual void Process(const char* Key, uint32_t& Data) = 0;
    virtual void Process(const char* Key, int16_t& Data) = 0;


    void Process(const char* Key, class RTTI*& Data);

    template <typename T>
    void Process(const char* Key, T& Data) 
    {
        Data.Process(*this);
    }

    template <typename T>
    void Process(const char* Key, std::vector<T>& Data) 
    {
        uint32_t size = static_cast<uint32_t>(Data.size());
        std::string sizeKey = Key ? std::string(Key) + "_size" : "size";
        Process(sizeKey.c_str(), size);

        if (IsReading()) {
            Data.resize(size);
        }

        for (uint32_t i = 0; i < size; ++i) {
            std::string elemKey = Key ? std::string(Key) + "_" + std::to_string(i) : std::to_string(i);
            Process(elemKey.c_str(), Data[i]);
        }
    }

    template <typename K, typename V>
    void Process(const char* Key, std::unordered_map<K, V>& Data) 
    {
        uint32_t size = static_cast<uint32_t>(Data.size());
        std::string sizeKey = Key ? std::string(Key) + "_size" : "size";
        Process(sizeKey.c_str(), size);

        if (IsReading()) 
        {
            Data.clear();
            for (uint32_t i = 0; i < size; ++i) 
            {
                K mapKey;
                V mapValue;
                std::string kKey = Key ? std::string(Key) + "_k" + std::to_string(i) : "k" + std::to_string(i);
                std::string vKey = Key ? std::string(Key) + "_v" + std::to_string(i) : "v" + std::to_string(i);

                Process(kKey.c_str(), mapKey);
                Process(vKey.c_str(), mapValue);
                Data[mapKey] = mapValue;
            }
        }
        else 
        {
            uint32_t i = 0;
            for (auto& pair : Data) 
            {
                K mapKey = pair.first;
                std::string kKey = Key ? std::string(Key) + "_k" + std::to_string(i) : "k" + std::to_string(i);
                std::string vKey = Key ? std::string(Key) + "_v" + std::to_string(i) : "v" + std::to_string(i);

                Process(kKey.c_str(), mapKey);
                Process(vKey.c_str(), pair.second);
                i++;
            }
        }
    }
};
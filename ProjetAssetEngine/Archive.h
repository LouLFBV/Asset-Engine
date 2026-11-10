#pragma once

#include <string>
#include <cstdint> 
#include <vector>       
#include <unordered_map>

enum class ArchiveDirection { Read, Write };

class Archive {
protected:
	ArchiveDirection m_direction;

public:
	Archive(ArchiveDirection dir) : m_direction(dir) {}
	virtual ~Archive() = default;

	inline bool IsReading() const { return m_direction == ArchiveDirection::Read; }
	inline bool IsWriting() const { return m_direction == ArchiveDirection::Write; }

	virtual void Process(const char* Key, int& Data) = 0;
	virtual void Process(const char* Key, float& Data) = 0;
	virtual void Process(const char* Key, std::string& Data) = 0;
	virtual void Process(const char* Key, double& Data) = 0;
	virtual void Process(const char* Key, bool& Data) = 0;
	virtual void Process(const char* Key, char& Data) = 0;

	virtual void Process(const char* Key, uint16_t& Data) = 0;
	virtual void Process(const char* Key, uint32_t& Data) = 0;
	virtual void Process(const char* Key, int16_t& Data) = 0;


    // DECLARATION pour les types polymorphes (RTTI)
    void Process(const char* Key, class RTTI*& Data);

    // TEMPLATE pour les structures et objets métiers (Pawn, Transform...)
    template <typename T>
    void Process(const char* Key, T& Data) {
        Data.Process(*this);
    }

    // TEMPLATE pour les tableaux (std::vector) et tableaux imbriqués
    template <typename T>
    void Process(const char* Key, std::vector<T>& Data) {
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

    // TEMPLATE pour les Maps (std::unordered_map)
    template <typename K, typename V>
    void Process(const char* Key, std::unordered_map<K, V>& Data) {
        uint32_t size = static_cast<uint32_t>(Data.size());
        std::string sizeKey = Key ? std::string(Key) + "_size" : "size";
        Process(sizeKey.c_str(), size);

        if (IsReading()) {
            Data.clear();
            for (uint32_t i = 0; i < size; ++i) {
                K mapKey;
                V mapValue;
                std::string kKey = Key ? std::string(Key) + "_k" + std::to_string(i) : "k" + std::to_string(i);
                std::string vKey = Key ? std::string(Key) + "_v" + std::to_string(i) : "v" + std::to_string(i);

                Process(kKey.c_str(), mapKey);
                Process(vKey.c_str(), mapValue);
                Data[mapKey] = mapValue;
            }
        }
        else {
            uint32_t i = 0;
            for (auto& pair : Data) {
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
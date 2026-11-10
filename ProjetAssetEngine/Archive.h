#pragma once

#include <string>

enum class ArchiveDirection { Read, Write };

class Archive {

protected:
	ArchiveDirection m_direction;

public:
	Archive(ArchiveDirection dir) : m_direction(dir) {}
	virtual ~Archive() = default;

	inline bool IsReading() const { m_direction == ArchiveDirection::Read; }
	inline bool IsWriting() const { m_direction == ArchiveDirection::Write; }


	// METHODES DE SERIALIZATION
	virtual void Serialize(const int& Key, float& Data) = 0;
	virtual void Serialize(const float& Key, float& Data) = 0;
	virtual void Serialize(const std::string& Key, float& Data) = 0;
	virtual void Serialize(const double& Key, float& Data) = 0;
	virtual void Serialize(const uint16_t& Key, float& Data) = 0;
	virtual void Serialize(const uint32_t& Key, float& Data) = 0;
	virtual void Serialize(const int16_t& Key, float& Data) = 0;
	virtual void Serialize(const int32_t& Key, float& Data) = 0;
	virtual void Serialize(const bool& Key, float& Data) = 0;
	virtual void Serialize(const char& Key, float& Data) = 0;

	// METHODES DE DESERIALISATION
	virtual void Deserialize(const int& Key, float& Data) = 0;
	virtual void Deserialize(const float& Key, float& Data) = 0;
	virtual void Deserialize(const std::string& Key, float& Data) = 0;
	virtual void Deserialize(const double& Key, float& Data) = 0;
	virtual void Deserialize(const uint16_t& Key, float& Data) = 0;
	virtual void Deserialize(const uint32_t& Key, float& Data) = 0;
	virtual void Deserialize(const int16_t& Key, float& Data) = 0;
	virtual void Deserialize(const int32_t& Key, float& Data) = 0;
	virtual void Deserialize(const bool& Key, float& Data) = 0;
	virtual void Deserialize(const char& Key, float& Data) = 0;
};
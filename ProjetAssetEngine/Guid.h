#pragma once

#include <random>

#include "Checksum.h"


struct Guid {
	unsigned long data1;
	unsigned short data2;
	unsigned short data3;
	unsigned char data4[8];

	bool operator==(const Guid& other) const;
	bool operator<(const Guid& other) const;

	static Guid Generate();
};

template<>
struct std::hash<Guid>
{
	static uint32_t operator()(const Guid& guid)
	{
		// On traite ton Guid comme un bloc brut de 16 octets (sizeof(Guid))
			// et on laisse ton Checksum faire la magie du CRC32 !
		return Checksum::CalculateCRC32(
			reinterpret_cast<const unsigned char*>(&guid),
			sizeof(Guid)
		);
	}
};
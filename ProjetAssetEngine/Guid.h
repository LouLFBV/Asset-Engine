#pragma once

#include <random>

#include "Checksum.h"

struct Guid {
	unsigned long data1;
	unsigned short data2;
	unsigned short data3;
	unsigned char data4[8];

	[[nodiscard]] bool operator==(const Guid& other) const noexcept;
	[[nodiscard]] bool operator<(const Guid& other) const noexcept;

	static Guid Generate();
};

template<>
struct std::hash<Guid>
{
	static inline uint32_t operator()(const Guid& guid)
	{
		return Checksum::CalculateCRC32(
			reinterpret_cast<const unsigned char*>(&guid),
			sizeof(Guid)
		);
	}
};
#include "Guid.h"
#include <limits>

bool Guid::operator==(const Guid& other) const
{
	if (data1 != other.data1 || data2 != other.data2 || data3 != other.data3)
		return false;

	for (int i = 0; i < 8; ++i)
		if (data4[i] != other.data4[i])
			return false;

	return true;
}

bool Guid::operator<(const Guid& other) const
{
	if (data1 < other.data1)
		return true;
	else if (data1 > other.data1)
		return false;

	if (data2 < other.data2)
		return true;
	else if (data2 > other.data2)
		return false;

	if (data3 < other.data3)
		return true;
	else if (data3 > other.data3)
		return false;

	for (int i = 0; i < 8; ++i) {
		if (data4[i] < other.data4[i])
			return true;
		if (data4[i] > other.data4[i])
			return false;
	}		

	return false;
}

Guid Guid::Generate()
{
	std::random_device rd;
	std::mt19937_64 gen(rd());

	Guid res;

	std::uniform_int_distribution<uint32_t> dist32(0, std::numeric_limits<uint32_t>::max());
	res.data1 = dist32(gen);

	std::uniform_int_distribution<uint16_t> dist16(0, std::numeric_limits<uint16_t>::max());
	res.data2 = dist16(gen);
	res.data3 = dist16(gen);

	std::uniform_int_distribution<int> dist8(0, std::numeric_limits<int8_t>::max());
	for (int i = 0; i < 8; ++i)
		res.data4[i] = dist8(gen);

	return res;
}
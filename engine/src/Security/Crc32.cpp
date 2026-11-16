#include <AssetEngine/Security/Crc32.h>

#include <array>

namespace AssetEngine
{
	namespace
	{
		// The CRC32 of each possible byte, computed by the compiler.
		constexpr std::array<std::uint32_t, 256> MakeTable()
		{
			constexpr std::uint32_t polynomial = 0xEDB88320u;

			std::array<std::uint32_t, 256> table{};
			for (std::uint32_t i = 0; i < 256; ++i)
			{
				std::uint32_t value = i;
				for (int bit = 0; bit < 8; ++bit)
					value = (value & 1u) ? (value >> 1) ^ polynomial : value >> 1;

				table[i] = value;
			}

			return table;
		}

		constexpr std::array<std::uint32_t, 256> Table = MakeTable();
	}

	void Crc32::Update(std::span<const std::byte> data)
	{
		for (const std::byte b : data)
			m_state = Table[(m_state ^ std::to_integer<std::uint32_t>(b)) & 0xFFu] ^ (m_state >> 8);
	}

	std::uint32_t Crc32::Compute(std::span<const std::byte> data)
	{
		Crc32 crc;
		crc.Update(data);
		return crc.Finish();
	}
}

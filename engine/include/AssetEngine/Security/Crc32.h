#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace AssetEngine
{
	// CRC32 (IEEE 802.3, the one of zip and png): detects accidental corruption.
	// It is not secure: anyone who changes the data can compute the new CRC32.
	class Crc32
	{
	public:
		static constexpr std::size_t DigestSize = 4;

		// Adds data to the checksum. Can be called several times, on consecutive pieces.
		void Update(std::span<const std::byte> data);

		// The checksum of all the data given to Update.
		std::uint32_t Finish() const { return ~m_state; }

		static std::uint32_t Compute(std::span<const std::byte> data);

	private:
		std::uint32_t m_state = 0xFFFFFFFFu;
	};
}

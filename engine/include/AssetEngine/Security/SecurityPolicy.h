#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace AssetEngine
{
	enum class IntegrityAlgorithm : std::uint8_t
	{
		Crc32,      // Accidental corruption only: debug builds, caches.
		HmacSha256  // Corruption and deliberate changes: release assets, saves.
	};

	// Decides how data is protected. The algorithm comes from the program, never from the file:
	// otherwise an attacker could ask for a CRC32, and compute it after changing the data.
	class SecurityPolicy
	{
	public:
		static SecurityPolicy Checksum();
		static SecurityPolicy Signed(std::vector<std::byte> key);

		IntegrityAlgorithm GetAlgorithm() const { return m_algorithm; }
		std::size_t GetDigestSize() const;

		std::vector<std::byte> ComputeDigest(std::span<const std::byte> data) const;
		bool Verify(std::span<const std::byte> data, std::span<const std::byte> digest) const;

		// Returns the data followed by its digest.
		std::vector<std::byte> Seal(std::span<const std::byte> data) const;

		// Returns the data without its digest, or nothing if the digest does not match.
		std::optional<std::span<const std::byte>> Open(std::span<const std::byte> sealed) const;

	private:
		SecurityPolicy(IntegrityAlgorithm algorithm, std::vector<std::byte> key);

		IntegrityAlgorithm m_algorithm;
		std::vector<std::byte> m_key;
	};
}

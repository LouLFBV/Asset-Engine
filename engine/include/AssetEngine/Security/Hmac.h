#pragma once

#include <AssetEngine/Security/Sha256.h>

#include <cstddef>
#include <span>

namespace AssetEngine
{
	// HMAC-SHA256 (RFC 2104): a hash that only someone who knows the key can compute.
	// Changing the data without the key cannot give a valid HMAC, unlike a CRC32.
	class HmacSha256
	{
	public:
		static constexpr std::size_t DigestSize = Sha256::DigestSize;
		using Digest = Sha256::Digest;

		static Digest Compute(std::span<const std::byte> key, std::span<const std::byte> data);

		// Compares in constant time.
		static bool Verify(std::span<const std::byte> key, std::span<const std::byte> data, std::span<const std::byte> expected);
	};
}

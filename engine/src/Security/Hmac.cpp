#include <AssetEngine/Security/Hmac.h>

#include <AssetEngine/Security/ConstantTime.h>

#include <algorithm>
#include <array>

namespace AssetEngine
{
	namespace
	{
		// SHA-256 works on blocks of 64 bytes.
		constexpr std::size_t BlockSize = 64;
	}

	HmacSha256::Digest HmacSha256::Compute(std::span<const std::byte> key, std::span<const std::byte> data)
	{
		// The key is brought to exactly one block: hashed if too long, padded with zeros if too short.
		std::array<std::byte, BlockSize> blockKey{};
		if (key.size() > BlockSize)
		{
			const Digest hashedKey = Sha256::Compute(key);
			std::copy(hashedKey.begin(), hashedKey.end(), blockKey.begin());
		}
		else
		{
			std::copy(key.begin(), key.end(), blockKey.begin());
		}

		// Two different keys are derived from it, with the constants of the RFC.
		std::array<std::byte, BlockSize> innerKey{};
		std::array<std::byte, BlockSize> outerKey{};
		for (std::size_t i = 0; i < BlockSize; ++i)
		{
			innerKey[i] = blockKey[i] ^ std::byte{0x36};
			outerKey[i] = blockKey[i] ^ std::byte{0x5C};
		}

		// HMAC = SHA256(outerKey + SHA256(innerKey + data))
		Sha256 inner;
		inner.Update(innerKey);
		inner.Update(data);
		const Digest innerDigest = inner.Finish();

		Sha256 outer;
		outer.Update(outerKey);
		outer.Update(innerDigest);
		return outer.Finish();
	}

	bool HmacSha256::Verify(std::span<const std::byte> key, std::span<const std::byte> data, std::span<const std::byte> expected)
	{
		return ConstantTimeEquals(Compute(key, data), expected);
	}
}

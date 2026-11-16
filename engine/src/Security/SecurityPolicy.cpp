#include <AssetEngine/Security/SecurityPolicy.h>

#include <AssetEngine/Security/ConstantTime.h>
#include <AssetEngine/Security/Crc32.h>
#include <AssetEngine/Security/Hmac.h>

#include <utility>

namespace AssetEngine
{
	SecurityPolicy::SecurityPolicy(IntegrityAlgorithm algorithm, std::vector<std::byte> key)
		: m_algorithm(algorithm)
		, m_key(std::move(key))
	{
	}

	SecurityPolicy SecurityPolicy::Checksum()
	{
		return SecurityPolicy(IntegrityAlgorithm::Crc32, {});
	}

	SecurityPolicy SecurityPolicy::Signed(std::vector<std::byte> key)
	{
		return SecurityPolicy(IntegrityAlgorithm::HmacSha256, std::move(key));
	}

	std::size_t SecurityPolicy::GetDigestSize() const
	{
		return m_algorithm == IntegrityAlgorithm::Crc32 ? Crc32::DigestSize : HmacSha256::DigestSize;
	}

	std::vector<std::byte> SecurityPolicy::ComputeDigest(std::span<const std::byte> data) const
	{
		if (m_algorithm == IntegrityAlgorithm::HmacSha256)
		{
			const HmacSha256::Digest digest = HmacSha256::Compute(m_key, data);
			return {digest.begin(), digest.end()};
		}

		// The CRC32 is stored in little-endian, like every number of the binary format.
		const std::uint32_t crc = Crc32::Compute(data);
		return {std::byte(crc), std::byte(crc >> 8), std::byte(crc >> 16), std::byte(crc >> 24)};
	}

	bool SecurityPolicy::Verify(std::span<const std::byte> data, std::span<const std::byte> digest) const
	{
		return ConstantTimeEquals(ComputeDigest(data), digest);
	}

	std::vector<std::byte> SecurityPolicy::Seal(std::span<const std::byte> data) const
	{
		std::vector<std::byte> sealed(data.begin(), data.end());
		const std::vector<std::byte> digest = ComputeDigest(data);
		sealed.insert(sealed.end(), digest.begin(), digest.end());
		return sealed;
	}

	std::optional<std::span<const std::byte>> SecurityPolicy::Open(std::span<const std::byte> sealed) const
	{
		// The digest size comes from the policy, not from the file.
		const std::size_t digestSize = GetDigestSize();
		if (sealed.size() < digestSize)
			return std::nullopt;

		const auto data = sealed.first(sealed.size() - digestSize);
		const auto digest = sealed.last(digestSize);
		if (!Verify(data, digest))
			return std::nullopt;

		return data;
	}
}

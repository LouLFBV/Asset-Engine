#include <AssetEngine/Security/Sha256.h>

#include <picosha2.h>

#include <algorithm>

namespace AssetEngine
{
	Sha256::Sha256()
		: m_hasher(std::make_unique<picosha2::hash256_one_by_one>())
	{
	}

	// Written here, where hash256_one_by_one is a complete type that unique_ptr can delete.
	Sha256::~Sha256() = default;
	Sha256::Sha256(Sha256&&) noexcept = default;
	Sha256& Sha256::operator=(Sha256&&) noexcept = default;

	void Sha256::Update(std::span<const std::byte> data)
	{
		const auto* first = reinterpret_cast<const unsigned char*>(data.data());
		m_hasher->process(first, first + data.size());
	}

	Sha256::Digest Sha256::Finish()
	{
		m_hasher->finish();

		std::array<unsigned char, DigestSize> bytes{};
		m_hasher->get_hash_bytes(bytes.begin(), bytes.end());

		Digest digest{};
		std::transform(bytes.begin(), bytes.end(), digest.begin(), [](unsigned char b) { return std::byte{b}; });
		return digest;
	}

	Sha256::Digest Sha256::Compute(std::span<const std::byte> data)
	{
		Sha256 hasher;
		hasher.Update(data);
		return hasher.Finish();
	}
}

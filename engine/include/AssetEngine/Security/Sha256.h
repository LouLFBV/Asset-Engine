#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <span>

namespace picosha2
{
	class hash256_one_by_one;
}

namespace AssetEngine
{
	// SHA-256, computed by the PicoSHA2 library.
	// The library stays hidden in Sha256.cpp: it could be replaced without touching the rest of the engine.
	class Sha256
	{
	public:
		static constexpr std::size_t DigestSize = 32;
		using Digest = std::array<std::byte, DigestSize>;

		Sha256();
		~Sha256();

		Sha256(Sha256&&) noexcept;
		Sha256& operator=(Sha256&&) noexcept;

		// Adds data to the hash. Can be called several times, on consecutive pieces.
		void Update(std::span<const std::byte> data);

		// The hash of all the data given to Update. Call it once, at the end.
		Digest Finish();

		static Digest Compute(std::span<const std::byte> data);

	private:
		std::unique_ptr<picosha2::hash256_one_by_one> m_hasher;
	};
}

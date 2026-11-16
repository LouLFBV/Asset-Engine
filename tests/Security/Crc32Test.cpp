#include "SecurityTestUtils.h"

#include <AssetEngine/Security/Crc32.h>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <vector>

using namespace AssetEngine;
using namespace AssetEngine::Tests;

TEST_CASE("CRC32 matches the official check value")
{
	// The check value of every CRC32 implementation.
	REQUIRE(Crc32::Compute(AsBytes("123456789")) == 0xCBF43926u);
	REQUIRE(Crc32::Compute(AsBytes("")) == 0x00000000u);
	REQUIRE(Crc32::Compute(AsBytes("The quick brown fox jumps over the lazy dog")) == 0x414FA339u);
}

TEST_CASE("CRC32 gives the same result in one piece or several")
{
	Crc32 crc;
	crc.Update(AsBytes("1234"));
	crc.Update(AsBytes(""));
	crc.Update(AsBytes("56789"));

	REQUIRE(crc.Finish() == 0xCBF43926u);
}

TEST_CASE("CRC32 changes when a single bit changes")
{
	std::vector<std::byte> data(1000, std::byte{0x42});
	const std::uint32_t original = Crc32::Compute(data);

	data[500] ^= std::byte{0x01};

	REQUIRE(Crc32::Compute(data) != original);
}

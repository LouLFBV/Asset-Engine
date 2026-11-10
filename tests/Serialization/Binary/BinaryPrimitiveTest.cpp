#include "BinaryTestUtils.h"

#include <cstdint>
#include <limits>
#include <string>

using namespace AssetEngine;
using namespace AssetEngine::Tests;

static_assert(!BinaryWriter::Loading);
static_assert(BinaryReader::Loading);

TEST_CASE("Binary archive round-trips booleans and integers")
{
	REQUIRE(BinaryRoundTrip(true) == true);
	REQUIRE(BinaryRoundTrip(false) == false);

	REQUIRE(BinaryRoundTrip(std::int32_t{-42}) == -42);
	REQUIRE(BinaryRoundTrip(std::numeric_limits<std::int32_t>::min()) == std::numeric_limits<std::int32_t>::min());
	REQUIRE(BinaryRoundTrip(std::numeric_limits<std::int32_t>::max()) == std::numeric_limits<std::int32_t>::max());
	REQUIRE(BinaryRoundTrip(std::numeric_limits<std::uint32_t>::max()) == std::numeric_limits<std::uint32_t>::max());
	REQUIRE(BinaryRoundTrip(std::numeric_limits<std::int64_t>::min()) == std::numeric_limits<std::int64_t>::min());
	REQUIRE(BinaryRoundTrip(std::numeric_limits<std::uint64_t>::max()) == std::numeric_limits<std::uint64_t>::max());
}

TEST_CASE("Binary archive round-trips floating point numbers")
{
	REQUIRE(BinaryRoundTrip(0.1f) == 0.1f);
	REQUIRE(BinaryRoundTrip(-2.5f) == -2.5f);
	REQUIRE(BinaryRoundTrip(0.1) == 0.1);
	REQUIRE(BinaryRoundTrip(std::numeric_limits<double>::max()) == std::numeric_limits<double>::max());
}

TEST_CASE("Binary archive round-trips strings")
{
	REQUIRE(BinaryRoundTrip(std::string{}).empty());
	REQUIRE(BinaryRoundTrip(std::string("Lyra")) == "Lyra");

	// The UTF-8 bytes of "Éléa": the archive copies bytes, so any encoding survives.
	const std::string accents = "\xC3\x89l\xC3\xA9" "a";
	REQUIRE(BinaryRoundTrip(accents) == accents);

	const std::string longText(1000, 'x');
	REQUIRE(BinaryRoundTrip(longText) == longText);
}

TEST_CASE("Binary archive writes little-endian integers and varint sizes")
{
	REQUIRE(BinaryBytes(std::int32_t{1}) == Bytes({0x01, 0x00, 0x00, 0x00}));
	REQUIRE(BinaryBytes(std::int32_t{-1}) == Bytes({0xFF, 0xFF, 0xFF, 0xFF}));
	REQUIRE(BinaryBytes(true) == Bytes({0x01}));
	REQUIRE(BinaryBytes(std::string("AB")) == Bytes({0x02, 0x41, 0x42}));

	// 300 does not fit in 7 bits: its varint takes 2 bytes, 0xAC then 0x02.
	const auto longText = BinaryBytes(std::string(300, 'x'));
	REQUIRE(longText.size() == 302);
	REQUIRE(longText[0] == std::byte{0xAC});
	REQUIRE(longText[1] == std::byte{0x02});
}

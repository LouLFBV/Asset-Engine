#include "SecurityTestUtils.h"

#include <AssetEngine/Security/Crc32.h>
#include <AssetEngine/Security/Hmac.h>
#include <AssetEngine/Security/SecurityPolicy.h>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <vector>

using namespace AssetEngine;
using namespace AssetEngine::Tests;

namespace
{
	std::vector<std::byte> Key(std::string_view text)
	{
		const auto bytes = AsBytes(text);
		return {bytes.begin(), bytes.end()};
	}

	bool SameBytes(std::span<const std::byte> a, std::span<const std::byte> b)
	{
		return std::ranges::equal(a, b);
	}
}

TEST_CASE("A sealed block opens with the same policy")
{
	const auto data = AsBytes("hp = 10, gold = 100");

	SECTION("CRC32")
	{
		const SecurityPolicy policy = SecurityPolicy::Checksum();
		const auto sealed = policy.Seal(data);

		REQUIRE(sealed.size() == data.size() + Crc32::DigestSize);

		const auto opened = policy.Open(sealed);
		REQUIRE(opened.has_value());
		REQUIRE(SameBytes(*opened, data));
	}

	SECTION("HMAC-SHA256")
	{
		const SecurityPolicy policy = SecurityPolicy::Signed(Key("secret"));
		const auto sealed = policy.Seal(data);

		REQUIRE(sealed.size() == data.size() + HmacSha256::DigestSize);

		const auto opened = policy.Open(sealed);
		REQUIRE(opened.has_value());
		REQUIRE(SameBytes(*opened, data));
	}
}

TEST_CASE("A modified byte is refused")
{
	for (const SecurityPolicy& policy : {SecurityPolicy::Checksum(), SecurityPolicy::Signed(Key("secret"))})
	{
		auto sealed = policy.Seal(AsBytes("hp = 10, gold = 100"));

		// One byte of the data, then one byte of the digest.
		auto badData = sealed;
		badData[3] ^= std::byte{0x01};
		REQUIRE_FALSE(policy.Open(badData).has_value());

		auto badDigest = sealed;
		badDigest.back() ^= std::byte{0x01};
		REQUIRE_FALSE(policy.Open(badDigest).has_value());
	}
}

TEST_CASE("A recomputed CRC32 is refused when an HMAC is required")
{
	const SecurityPolicy release = SecurityPolicy::Signed(Key("secret"));

	// The attacker changes the data, then seals it again with the only algorithm they can compute.
	const auto forged = SecurityPolicy::Checksum().Seal(AsBytes("hp = 10, gold = 999999"));

	REQUIRE_FALSE(release.Open(forged).has_value());
}

TEST_CASE("A wrong key is refused")
{
	const auto sealed = SecurityPolicy::Signed(Key("secret")).Seal(AsBytes("save data"));

	REQUIRE_FALSE(SecurityPolicy::Signed(Key("guess")).Open(sealed).has_value());
}

TEST_CASE("A block shorter than a digest is refused")
{
	REQUIRE_FALSE(SecurityPolicy::Checksum().Open(Repeat(0x00, 3)).has_value());
	REQUIRE_FALSE(SecurityPolicy::Signed(Key("secret")).Open(Repeat(0x00, 31)).has_value());
}

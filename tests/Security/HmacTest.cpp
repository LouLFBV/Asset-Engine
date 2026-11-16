#include "SecurityTestUtils.h"

#include <AssetEngine/Security/ConstantTime.h>
#include <AssetEngine/Security/Hmac.h>

#include <catch2/catch_test_macros.hpp>

using namespace AssetEngine;
using namespace AssetEngine::Tests;

// Official test vectors, from RFC 4231.
TEST_CASE("HMAC-SHA256 matches the official test vectors")
{
	SECTION("Test case 1: short key")
	{
		REQUIRE(ToHex(HmacSha256::Compute(Repeat(0x0B, 20), AsBytes("Hi There")))
			== "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7");
	}

	SECTION("Test case 2: key shorter than the digest")
	{
		REQUIRE(ToHex(HmacSha256::Compute(AsBytes("Jefe"), AsBytes("what do ya want for nothing?")))
			== "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843");
	}

	SECTION("Test case 3: data made of 0xDD bytes")
	{
		REQUIRE(ToHex(HmacSha256::Compute(Repeat(0xAA, 20), Repeat(0xDD, 50)))
			== "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe");
	}

	SECTION("Test case 6: key longer than a block, hashed first")
	{
		REQUIRE(ToHex(HmacSha256::Compute(Repeat(0xAA, 131), AsBytes("Test Using Larger Than Block-Size Key - Hash Key First")))
			== "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54");
	}
}

TEST_CASE("HMAC-SHA256 refuses a wrong key or modified data")
{
	const auto key = AsBytes("the right key");
	const auto data = AsBytes("gold = 100");
	const HmacSha256::Digest digest = HmacSha256::Compute(key, data);

	REQUIRE(HmacSha256::Verify(key, data, digest));
	REQUIRE_FALSE(HmacSha256::Verify(AsBytes("the wrong key"), data, digest));
	REQUIRE_FALSE(HmacSha256::Verify(key, AsBytes("gold = 999"), digest));
}

TEST_CASE("Constant-time comparison")
{
	REQUIRE(ConstantTimeEquals(AsBytes("abcd"), AsBytes("abcd")));
	REQUIRE_FALSE(ConstantTimeEquals(AsBytes("abcd"), AsBytes("abce")));
	REQUIRE_FALSE(ConstantTimeEquals(AsBytes("abcd"), AsBytes("abc")));
	REQUIRE(ConstantTimeEquals(AsBytes(""), AsBytes("")));
}

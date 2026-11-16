#include "SecurityTestUtils.h"

#include <AssetEngine/Security/Sha256.h>

#include <catch2/catch_test_macros.hpp>

#include <string>

using namespace AssetEngine;
using namespace AssetEngine::Tests;

// Official test vectors, from the NIST examples of FIPS 180-2.
TEST_CASE("SHA-256 matches the official test vectors")
{
	REQUIRE(ToHex(Sha256::Compute(AsBytes(""))) == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
	REQUIRE(ToHex(Sha256::Compute(AsBytes("abc"))) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
	REQUIRE(ToHex(Sha256::Compute(AsBytes("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")))
		== "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
}

TEST_CASE("SHA-256 of one million 'a'")
{
	const std::string million(1'000'000, 'a');
	REQUIRE(ToHex(Sha256::Compute(AsBytes(million))) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
}

TEST_CASE("SHA-256 gives the same result in one piece or several")
{
	Sha256 hasher;
	hasher.Update(AsBytes("ab"));
	hasher.Update(AsBytes(""));
	hasher.Update(AsBytes("c"));

	REQUIRE(hasher.Finish() == Sha256::Compute(AsBytes("abc")));
}

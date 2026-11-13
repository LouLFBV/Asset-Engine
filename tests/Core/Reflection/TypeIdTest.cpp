#include <AssetEngine/Core/Reflection/TypeId.h>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>
#include <unordered_map>

using namespace AssetEngine;

namespace
{
	struct Warrior
	{
		static constexpr std::string_view TypeName = "Warrior";
	};

	struct Mage
	{
		static constexpr std::string_view TypeName = "Mage";
	};

	struct Unnamed
	{
	};

	// Official FNV-1a 64-bit test values.
	static_assert(Fnv1a64("") == 0xcbf29ce484222325ull);
	static_assert(Fnv1a64("a") == 0xaf63dc4c8601ec8cull);
	static_assert(Fnv1a64("foobar") == 0x85944171f73967e8ull);

	// These lines only compile because the ids are computed by the compiler.
	static_assert(TypeIdOf<Warrior> == TypeIdFromName("Warrior"));
	static_assert(TypeIdOf<Warrior> != TypeIdOf<Mage>);

	static_assert(Named<Warrior>);
	static_assert(!Named<Unnamed>);
}

TEST_CASE("Type ids depend only on the name")
{
	REQUIRE(TypeIdFromName("Warrior") == TypeIdOf<Warrior>);
	REQUIRE(TypeIdFromName("Mage") == TypeIdOf<Mage>);
	REQUIRE(TypeIdOf<Warrior> != TypeIdOf<Mage>);
}

TEST_CASE("Type ids computed at run time match the compile-time ones")
{
	// A name read from a file only exists at run time: it must give the same id.
	const std::string nameFromFile = "Warrior";
	REQUIRE(TypeIdFromName(nameFromFile) == TypeIdOf<Warrior>);
}

TEST_CASE("Type ids can be used as map keys")
{
	std::unordered_map<TypeId, std::string> names;
	names[TypeIdOf<Warrior>] = "Warrior";
	names[TypeIdOf<Mage>] = "Mage";

	REQUIRE(names.size() == 2);
	REQUIRE(names.at(TypeIdFromName("Mage")) == "Mage");
}

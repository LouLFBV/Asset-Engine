#include <AssetEngine/Core/Reflection/TypeRegistry.h>

#include <catch2/catch_test_macros.hpp>

using namespace AssetEngine;

namespace
{
	class Potion
	{
		ASSET_REFLECT(Potion, 1)

		virtual ~Potion() = default;
	};

	class Elixir
	{
		ASSET_REFLECT(Elixir, 1)

		virtual ~Elixir() = default;
	};
}

// Runs before main: Potion is already in the global registry when the tests start.
ASSET_REGISTER_TYPE(Potion)

TEST_CASE("A registered type exists as soon as main starts")
{
	const TypeRegistry& registry = TypeRegistry::Global();

	REQUIRE(registry.Find(TypeIdOf<Potion>) == &TypeOf<Potion>());
	REQUIRE(registry.Find("Potion") == &TypeOf<Potion>());
	REQUIRE(registry.Find("Unknown") == nullptr);
}

TEST_CASE("A frozen registry refuses new types")
{
	TypeRegistry registry;
	REQUIRE(registry.Register(TypeOf<Potion>()));
	REQUIRE_FALSE(registry.IsFrozen());

	registry.Freeze();

	REQUIRE(registry.IsFrozen());
	REQUIRE_FALSE(registry.Register(TypeOf<Elixir>()));
	REQUIRE(registry.Find("Elixir") == nullptr);

	// Reading still works after the freeze.
	REQUIRE(registry.Find("Potion") == &TypeOf<Potion>());
}

TEST_CASE("Registering the same type twice is harmless")
{
	TypeRegistry registry;
	REQUIRE(registry.Register(TypeOf<Potion>()));
	REQUIRE(registry.Register(TypeOf<Potion>()));
	REQUIRE(registry.Count() == 1);
}

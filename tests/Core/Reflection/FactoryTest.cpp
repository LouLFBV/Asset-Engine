#include <AssetEngine/Core/Reflection/Factory.h>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>

using namespace AssetEngine;

namespace
{
	class Monster
	{
		ASSET_REFLECT(Monster, 1)

		virtual ~Monster() = default;
		virtual std::int32_t Damage() const { return 1; }
	};

	class Goblin : public Monster
	{
		ASSET_REFLECT(Goblin, 1, Monster)

		std::int32_t Damage() const override { return 5; }
	};

	class Titled
	{
		ASSET_REFLECT(Titled, 1)

		virtual ~Titled() = default;
		std::string label = "nameless";
	};

	// Monster is its second base: creating it as a Monster must move the pointer.
	class Boss : public Titled, public Monster
	{
		ASSET_REFLECT(Boss, 1, Titled, Monster)

		std::int32_t Damage() const override { return 50; }
	};

	class Trap
	{
		ASSET_REFLECT(Trap, 1)

		virtual ~Trap() = default;
	};

	class Spell
	{
		ASSET_REFLECT(Spell, 1)

		virtual ~Spell() = default;
		virtual std::int32_t Cost() const = 0;
	};
}

ASSET_REGISTER_TYPE(Goblin)
ASSET_REGISTER_TYPE(Boss)
ASSET_REGISTER_TYPE(Trap)
ASSET_REGISTER_TYPE(Spell)

TEST_CASE("Factory creates a type from its id")
{
	std::unique_ptr<Monster> monster = Factory::Create<Monster>(TypeIdOf<Goblin>);

	REQUIRE(monster != nullptr);
	REQUIRE(monster->Damage() == 5);
	REQUIRE(monster->GetTypeInfo().GetName() == "Goblin");
}

TEST_CASE("Factory creates a type from a name read at run time")
{
	const std::string nameFromFile = "Goblin";
	std::unique_ptr<Monster> monster = Factory::Create<Monster>(TypeIdFromName(nameFromFile));

	REQUIRE(monster != nullptr);
	REQUIRE(monster->Damage() == 5);
}

TEST_CASE("Factory returns a correct pointer to a second base")
{
	std::unique_ptr<Monster> monster = Factory::Create<Monster>(TypeIdOf<Boss>);

	REQUIRE(monster != nullptr);
	REQUIRE(monster->Damage() == 50);

	// Back to the real type: the Titled part is intact, so the pointer was moved correctly.
	const Boss* boss = dynamic_cast<const Boss*>(monster.get());
	REQUIRE(boss != nullptr);
	REQUIRE(boss->label == "nameless");
}

TEST_CASE("Factory refuses what it cannot create")
{
	REQUIRE(Factory::Create<Monster>(TypeIdFromName("Unknown")) == nullptr);

	// Trap is registered, but it is not a Monster.
	REQUIRE(Factory::Create<Monster>(TypeIdOf<Trap>) == nullptr);

	// Spell is registered, but it is abstract.
	REQUIRE(Factory::Create<Spell>(TypeIdOf<Spell>) == nullptr);
}

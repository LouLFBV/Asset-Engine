#include <AssetEngine/Core/IO/MemoryStream.h>
#include <AssetEngine/Core/Reflection/TypeRegistry.h>
#include <AssetEngine/Serialization/Binary/BinaryReader.h>
#include <AssetEngine/Serialization/Binary/BinaryWriter.h>
#include <AssetEngine/Serialization/Json/JsonReader.h>
#include <AssetEngine/Serialization/Json/JsonWriter.h>
#include <AssetEngine/Serialization/Serialization.h>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

using namespace AssetEngine;

namespace
{
	class Weapon
	{
		ASSET_REFLECT(Weapon, 1)

		virtual ~Weapon() = default;

		virtual void Serialize(Archive& archive)
		{
			archive.Value("damage", damage);
		}

		std::int32_t damage = 1;
	};

	class Sword : public Weapon
	{
		ASSET_REFLECT(Sword, 1, Weapon)

		void Serialize(Archive& archive) override
		{
			Weapon::Serialize(archive);
			archive.Value("sharpness", sharpness);
		}

		float sharpness = 0.5f;
	};

	class Bow : public Weapon
	{
		ASSET_REFLECT(Bow, 1, Weapon)

		void Serialize(Archive& archive) override
		{
			Weapon::Serialize(archive);
			archive.Value("arrows", arrows);
		}

		std::int32_t arrows = 10;
	};

	class Shield
	{
		ASSET_REFLECT(Shield, 1)

		virtual ~Shield() = default;
		virtual void Serialize(Archive&) {}
	};

	struct Inventory
	{
		std::vector<std::unique_ptr<Weapon>> weapons;
		std::unique_ptr<Weapon> empty;

		void Serialize(Archive& archive)
		{
			archive.Value("weapons", weapons);
			archive.Value("empty", empty);
		}
	};

	Inventory MakeInventory()
	{
		Inventory inventory;

		auto sword = std::make_unique<Sword>();
		sword->damage = 12;
		sword->sharpness = 0.9f;
		inventory.weapons.push_back(std::move(sword));

		auto bow = std::make_unique<Bow>();
		bow->damage = 7;
		bow->arrows = 30;
		inventory.weapons.push_back(std::move(bow));

		return inventory;
	}

	void CheckInventory(const Inventory& inventory)
	{
		REQUIRE(inventory.weapons.size() == 2);
		REQUIRE(inventory.empty == nullptr);

		// Each weapon came back as its real type, with its own fields.
		const auto* sword = dynamic_cast<const Sword*>(inventory.weapons[0].get());
		REQUIRE(sword != nullptr);
		REQUIRE(sword->damage == 12);
		REQUIRE(sword->sharpness == 0.9f);

		const auto* bow = dynamic_cast<const Bow*>(inventory.weapons[1].get());
		REQUIRE(bow != nullptr);
		REQUIRE(bow->damage == 7);
		REQUIRE(bow->arrows == 30);
	}

	ArchiveError LoadFromJson(const std::string& text, Inventory& inventory)
	{
		MemoryStream stream;
		REQUIRE(stream.Write(std::as_bytes(std::span(text.data(), text.size()))));
		REQUIRE(stream.Seek(0));

		JsonReader reader(stream);
		reader.Value("inventory", inventory);
		return reader.GetError();
	}
}

ASSET_REGISTER_TYPE(Sword)
ASSET_REGISTER_TYPE(Bow)
ASSET_REGISTER_TYPE(Shield)

TEST_CASE("Binary archive recreates the real type behind a base pointer")
{
	Inventory saved = MakeInventory();

	MemoryStream stream;
	BinaryWriter writer(stream);
	writer.Value("inventory", saved);
	REQUIRE_FALSE(writer.HasFailed());

	REQUIRE(stream.Seek(0));
	Inventory loaded;
	BinaryReader reader(stream);
	reader.Value("inventory", loaded);
	REQUIRE_FALSE(reader.HasFailed());

	CheckInventory(loaded);
}

TEST_CASE("JSON archive recreates the real type behind a base pointer")
{
	Inventory saved = MakeInventory();

	MemoryStream stream;
	JsonWriter writer(stream);
	writer.Value("inventory", saved);
	REQUIRE(writer.Finish());

	const auto bytes = stream.Data();
	const std::string text(reinterpret_cast<const char*>(bytes.data()), bytes.size());
	REQUIRE(text.find("\"type\": \"Sword\"") != std::string::npos);

	Inventory loaded;
	REQUIRE(LoadFromJson(text, loaded) == ArchiveError::None);
	CheckInventory(loaded);
}

TEST_CASE("Polymorphic loading refuses an unknown type")
{
	Inventory inventory;
	const std::string text = R"({ "inventory": { "weapons": [ { "type": "Axe", "data": {} } ], "empty": { "type": "" } } })";
	REQUIRE(LoadFromJson(text, inventory) == ArchiveError::InvalidData);
}

TEST_CASE("Polymorphic loading refuses a type that is not a Weapon")
{
	Inventory inventory;
	const std::string text = R"({ "inventory": { "weapons": [ { "type": "Shield", "data": {} } ], "empty": { "type": "" } } })";
	REQUIRE(LoadFromJson(text, inventory) == ArchiveError::InvalidData);
}

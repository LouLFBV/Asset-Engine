#include "AssetTestUtils.h"

#include <AssetEngine/Asset/AssetManager.h>
#include <AssetEngine/Core/Reflection/TypeRegistry.h>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <string>

using namespace AssetEngine;
using namespace AssetEngine::Tests;

namespace
{
	class Material : public Asset
	{
		ASSET_REFLECT(Material, 1, Asset)

		void Serialize(Archive& archive) override
		{
			archive.Value("color", color);
			archive.Value("roughness", roughness);
		}

		std::string color;
		float roughness = 0.0f;
	};

	class Shader : public Asset
	{
		ASSET_REFLECT(Shader, 1, Asset)

		void Serialize(Archive& archive) override { archive.Value("code", code); }

		std::string code;
	};

	// A folder with one material and one shader, already scanned.
	struct Fixture
	{
		Fixture()
		{
			Material material;
			material.color = "red";
			material.roughness = 0.25f;
			WriteAssetFile(folder.Path() / "materials/red.asset", material);

			Shader shader;
			shader.code = "void main() {}";
			WriteAssetFile(folder.Path() / "shaders/basic.asset", shader, {}, AssetEncoding::Json);

			registry.Scan();
		}

		TempDirectory folder;
		AssetRegistry registry{folder.Path()};
	};
}

ASSET_REGISTER_TYPE(Material)
ASSET_REGISTER_TYPE(Shader)

TEST_CASE("Two loads give the same instance")
{
	Fixture fixture;
	AssetManager manager(fixture.registry, SecurityPolicy::Checksum());

	const AssetHandle<Asset> first = manager.Load("materials/red.asset");
	const AssetHandle<Asset> second = manager.Load("Materials\\Red.asset");

	REQUIRE(first);
	REQUIRE(first.Get() == second.Get());
	REQUIRE(manager.GetLoadedCount() == 1);
}

TEST_CASE("An asset is freed with its last handle")
{
	Fixture fixture;
	AssetManager manager(fixture.registry, SecurityPolicy::Checksum());
	const AssetId id = AssetIdFromPath("materials/red.asset");

	{
		const AssetHandle<Asset> handle = manager.Load(id);
		const AssetHandle<Asset> copy = handle;
		REQUIRE(manager.IsLoaded(id));
	}

	REQUIRE_FALSE(manager.IsLoaded(id));
	REQUIRE(manager.GetLoadedCount() == 0);

	// Loading it again reads the file again.
	const AssetHandle<Material> reloaded = manager.Load<Material>(id);
	REQUIRE(reloaded);
	REQUIRE(reloaded->color == "red");
	REQUIRE(manager.IsLoaded(id));
}

TEST_CASE("A typed load checks the type of the asset")
{
	Fixture fixture;
	AssetManager manager(fixture.registry, SecurityPolicy::Checksum());

	const AssetHandle<Material> material = manager.Load<Material>("materials/red.asset");
	REQUIRE(material);
	REQUIRE(material->roughness == 0.25f);

	const AssetHandle<Shader> shader = manager.Load<Shader>("shaders/basic.asset");
	REQUIRE(shader);
	REQUIRE(shader->code == "void main() {}");

	const AssetHandle<Shader> wrong = manager.Load<Shader>("materials/red.asset");
	REQUIRE_FALSE(wrong);
	REQUIRE(wrong.GetError() == AssetError::WrongType);
}

TEST_CASE("A failed load tells why")
{
	Fixture fixture;

	SECTION("Unknown path")
	{
		AssetManager manager(fixture.registry, SecurityPolicy::Checksum());
		REQUIRE(manager.Load("missing.asset").GetError() == AssetError::NotInRegistry);
	}

	SECTION("File deleted after the scan")
	{
		AssetManager manager(fixture.registry, SecurityPolicy::Checksum());
		RemoveFile(fixture.folder.Path() / "materials/red.asset");
		REQUIRE(manager.Load("materials/red.asset").GetError() == AssetError::FileNotFound);
	}

	SECTION("File protected by another policy")
	{
		AssetManager manager(fixture.registry, SecurityPolicy::Signed({std::byte{42}}));
		REQUIRE(manager.Load("materials/red.asset").GetError() == AssetError::IntegrityFailure);
		REQUIRE(manager.GetLoadedCount() == 0);
	}
}

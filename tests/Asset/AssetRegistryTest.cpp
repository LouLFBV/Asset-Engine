#include "AssetTestUtils.h"

#include <AssetEngine/Asset/AssetRegistry.h>
#include <AssetEngine/Core/Reflection/TypeRegistry.h>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace AssetEngine;
using namespace AssetEngine::Tests;

namespace
{
	class Sound : public Asset
	{
		ASSET_REFLECT(Sound, 1, Asset)

		void Serialize(Archive& archive) override { archive.Value("duration", duration); }

		float duration = 1.5f;
	};

	class Mesh : public Asset
	{
		ASSET_REFLECT(Mesh, 1, Asset)

		void Serialize(Archive& archive) override { archive.Value("vertices", vertices); }

		std::vector<float> vertices{0.0f, 1.0f, 2.0f};
	};

	// Three valid assets in two folders, one broken .asset file and one file that is not an asset.
	void FillFolder(const std::filesystem::path& root)
	{
		Sound jump;
		Sound music;
		Mesh hero;

		WriteAssetFile(root / "sounds/jump.asset", jump, {1, {}, {"sfx"}});
		WriteAssetFile(root / "sounds/music.asset", music, {1, {}, {"music", "level1"}});
		WriteAssetFile(root / "meshes/hero.asset", hero, {2, {"sounds/jump.asset"}, {"level1"}}, AssetEncoding::Json);

		std::ofstream(root / "broken.asset") << "not an asset";
		std::ofstream(root / "readme.txt") << "ignored";
	}

	std::vector<std::string> Paths(const std::vector<const AssetRecord*>& records)
	{
		std::vector<std::string> paths;
		for (const AssetRecord* record : records)
			paths.push_back(record->path);

		return paths;
	}
}

ASSET_REGISTER_TYPE(Sound)
ASSET_REGISTER_TYPE(Mesh)

TEST_CASE("Asset ids ignore slashes and letter case")
{
	REQUIRE(AssetIdFromPath("Sounds\\Jump.asset") == AssetIdFromPath("sounds/jump.asset"));
	REQUIRE(AssetIdFromPath("./sounds/jump.asset") == AssetIdFromPath("sounds/jump.asset"));
	REQUIRE(AssetIdFromPath("sounds/jump.asset") != AssetIdFromPath("sounds/music.asset"));
}

TEST_CASE("Scan finds every asset of the folder")
{
	TempDirectory folder;
	FillFolder(folder.Path());

	AssetRegistry registry(folder.Path());
	const ScanReport report = registry.Scan();

	REQUIRE(report.found == 3);
	REQUIRE(registry.Count() == 3);

	// The broken file is reported, the text file is ignored.
	REQUIRE(report.errors.size() == 1);
	REQUIRE(report.errors[0].starts_with("broken.asset"));

	REQUIRE(Paths(registry.GetAll()) == std::vector<std::string>{"meshes/hero.asset", "sounds/jump.asset", "sounds/music.asset"});
}

TEST_CASE("The registry finds assets by path, type and tag")
{
	TempDirectory folder;
	FillFolder(folder.Path());

	AssetRegistry registry(folder.Path());
	registry.Scan();

	const AssetRecord* hero = registry.Find("Meshes\\Hero.asset");
	REQUIRE(hero != nullptr);
	REQUIRE(hero->path == "meshes/hero.asset");
	REQUIRE(hero->type == TypeIdOf<Mesh>);
	REQUIRE(hero->encoding == AssetEncoding::Json);
	REQUIRE(hero->metadata.version == 2);
	REQUIRE(hero->metadata.dependencies == std::vector<std::string>{"sounds/jump.asset"});
	REQUIRE(std::filesystem::exists(registry.GetFullPath(*hero)));

	REQUIRE(registry.Find("missing.asset") == nullptr);

	REQUIRE(Paths(registry.FindByType(TypeIdOf<Sound>)) == std::vector<std::string>{"sounds/jump.asset", "sounds/music.asset"});
	REQUIRE(Paths(registry.FindByTag("level1")) == std::vector<std::string>{"meshes/hero.asset", "sounds/music.asset"});
	REQUIRE(registry.FindByTag("unknown").empty());
}

TEST_CASE("registry.bin restores the registry without scanning")
{
	TempDirectory folder;
	FillFolder(folder.Path());
	const auto cache = folder.Path() / "registry.bin";

	AssetRegistry scanned(folder.Path());
	scanned.Scan();
	REQUIRE(scanned.SaveCache(cache));

	AssetRegistry restored(folder.Path());
	REQUIRE(restored.LoadCache(cache));

	REQUIRE(restored.Count() == 3);
	REQUIRE(Paths(restored.GetAll()) == Paths(scanned.GetAll()));
	REQUIRE(restored.Find("meshes/hero.asset")->metadata == scanned.Find("meshes/hero.asset")->metadata);
	REQUIRE(restored.Find("sounds/music.asset")->type == TypeIdOf<Sound>);
}

TEST_CASE("A damaged registry.bin is refused, then rebuilt by Scan")
{
	TempDirectory folder;
	FillFolder(folder.Path());
	const auto cache = folder.Path() / "registry.bin";

	AssetRegistry registry(folder.Path());
	registry.Scan();
	REQUIRE(registry.SaveCache(cache));

	// One bit of the cache changes.
	FlipBit(cache, 10);

	AssetRegistry restored(folder.Path());
	REQUIRE_FALSE(restored.LoadCache(cache));
	REQUIRE_FALSE(restored.LoadCache(folder.Path() / "missing.bin"));
	REQUIRE(restored.Count() == 0);

	// The .asset files are the reference: scanning them rebuilds the registry and its cache.
	REQUIRE(restored.Scan().found == 3);
	REQUIRE(restored.SaveCache(cache));
	REQUIRE(AssetRegistry(folder.Path()).LoadCache(cache));
}

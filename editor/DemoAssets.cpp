#include "DemoAssets.h"

#include "ImageImporter.h"

#include <AssetEngine/Asset/AssetFile.h>
#include <AssetEngine/Core/IO/FileOutputStream.h>
#include <AssetEngine/Core/Reflection/TypeRegistry.h>
#include <AssetEngine/Serialization/Serialization.h>

#include <string>

using namespace AssetEngine;

// Registered before main, like every type of the engine.
ASSET_REGISTER_TYPE(Demo::TextureAsset)
ASSET_REGISTER_TYPE(Demo::MaterialAsset)
ASSET_REGISTER_TYPE(Demo::ConfigAsset)

namespace Demo
{
	void TextureAsset::Serialize(Archive& archive)
	{
		archive.Value("width", width);
		archive.Value("height", height);
		archive.Value("pixels", pixels);
	}

	void MaterialAsset::Serialize(Archive& archive)
	{
		archive.Value("texture", texture);
		archive.Value("color", color);
		archive.Value("roughness", roughness);
	}

	void ConfigAsset::Serialize(Archive& archive)
	{
		archive.Value("values", values);
	}

	namespace
	{
		void Write(const std::filesystem::path& file, Asset& asset, const AssetMetadata& metadata, AssetEncoding encoding, const SecurityPolicy& policy)
		{
			std::filesystem::create_directories(file.parent_path());
			FileOutputStream stream(file);
			WriteAsset(stream, asset, metadata, encoding, policy);
		}

		TextureAsset MakeTexture(std::int32_t size, std::uint32_t seed)
		{
			TextureAsset texture;
			texture.width = size;
			texture.height = size;
			texture.pixels.resize(static_cast<std::size_t>(size) * size);
			for (std::size_t i = 0; i < texture.pixels.size(); ++i)
				texture.pixels[i] = 0xFF000000u | static_cast<std::uint32_t>(i * seed);

			return texture;
		}
	}

	void CreateDemoAssets(const std::filesystem::path& folder, const SecurityPolicy& policy)
	{
		if (std::filesystem::exists(folder))
			return;

		TextureAsset hero = MakeTexture(256, 7);
		Write(folder / "textures/hero.asset", hero, {1, {}, {"character"}}, AssetEncoding::Binary, policy);

		// About 4 MB: big enough to see the difference between a synchronous and an asynchronous load.
		TextureAsset landscape = MakeTexture(1024, 13);
		Write(folder / "textures/landscape.asset", landscape, {1, {}, {"level1"}}, AssetEncoding::Binary, policy);

		MaterialAsset material;
		material.texture = "textures/hero.asset";
		material.color = {1.0f, 0.8f, 0.6f, 1.0f};
		material.roughness = 0.3f;
		Write(folder / "materials/hero.asset", material, {2, {"textures/hero.asset"}, {"character"}}, AssetEncoding::Json, policy);

		ConfigAsset config;
		config.values = {{"title", "Asset Engine demo"}, {"difficulty", "normal"}, {"volume", "0.8"}};
		Write(folder / "config/game.asset", config, {1, {}, {"config"}}, AssetEncoding::Json, policy);

		for (int i = 1; i <= 12; ++i)
		{
			TextureAsset icon = MakeTexture(64, static_cast<std::uint32_t>(i));
			Write(folder / ("icons/icon" + std::to_string(i) + ".asset"), icon, {1, {}, {"ui"}}, AssetEncoding::Binary, policy);
		}
	}

	std::filesystem::path GetDemoImagePath()
	{
		return std::filesystem::path(EDITOR_IMAGES_DIR) / "dice.png";
	}

	bool ImportDemoImage(const std::filesystem::path& folder, const SecurityPolicy& policy)
	{
		const std::filesystem::path target = folder / "textures/dice.asset";
		if (std::filesystem::exists(target))
			return true;

		// From here, the real image follows the same path as every other asset.
		std::optional<TextureAsset> dice = ImportImage(GetDemoImagePath());
		if (!dice)
			return false;

		Write(target, *dice, {1, {}, {"imported"}}, AssetEncoding::Binary, policy);
		return true;
	}
}

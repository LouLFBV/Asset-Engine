#pragma once

#include <AssetEngine/Asset/Asset.h>
#include <AssetEngine/Security/SecurityPolicy.h>

#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

// Asset types for the demo. They live outside the engine: a real game would declare its own.
namespace Demo
{
	class TextureAsset : public AssetEngine::Asset
	{
		ASSET_REFLECT(TextureAsset, 1, AssetEngine::Asset)

		void Serialize(AssetEngine::Archive& archive) override;

		std::int32_t width = 0;
		std::int32_t height = 0;
		std::vector<std::uint32_t> pixels;
	};

	class MaterialAsset : public AssetEngine::Asset
	{
		ASSET_REFLECT(MaterialAsset, 1, AssetEngine::Asset)

		void Serialize(AssetEngine::Archive& archive) override;

		std::string texture;
		std::vector<float> color;
		float roughness = 0.5f;
	};

	class ConfigAsset : public AssetEngine::Asset
	{
		ASSET_REFLECT(ConfigAsset, 1, AssetEngine::Asset)

		void Serialize(AssetEngine::Archive& archive) override;

		std::map<std::string, std::string> values;
	};

	// Writes the demo assets into the folder, if it does not exist yet.
	void CreateDemoAssets(const std::filesystem::path& folder, const AssetEngine::SecurityPolicy& policy);

	// The real image of the demo, in editor/images.
	std::filesystem::path GetDemoImagePath();

	// Imports the real image into textures/dice.asset, if that file does not exist yet.
	// Returns false if the image could not be read.
	bool ImportDemoImage(const std::filesystem::path& folder, const AssetEngine::SecurityPolicy& policy);
}

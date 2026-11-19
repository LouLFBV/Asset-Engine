#pragma once

#include "DemoAssets.h"

#include <filesystem>
#include <optional>

namespace Demo
{
	// Reads a PNG or JPEG image and turns it into a TextureAsset: the "importer" of the engine.
	// A common format is read once, by this tool; the game then only reads .asset files.
	// Returns nothing if the file is missing or is not an image.
	std::optional<TextureAsset> ImportImage(const std::filesystem::path& file);
}

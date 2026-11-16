#pragma once

#include <AssetEngine/Serialization/Serialization.h>

#include <cstdint>
#include <string>
#include <vector>

namespace AssetEngine
{
	// What the registry knows about an asset without loading it.
	struct AssetMetadata
	{
		// The version of the content, raised each time the asset is saved again.
		std::uint32_t version = 1;

		// The paths of the assets this one needs, for example the textures of a material.
		std::vector<std::string> dependencies;

		// Free labels to search assets, for example "ui" or "level1".
		std::vector<std::string> tags;

		void Serialize(Archive& archive)
		{
			archive.Value("version", version);
			archive.Value("dependencies", dependencies);
			archive.Value("tags", tags);
		}

		bool operator==(const AssetMetadata&) const = default;
	};
}

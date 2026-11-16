#pragma once

#include <AssetEngine/Core/Hash.h>

#include <cctype>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

namespace AssetEngine
{
	// The identifier of an asset: the hash of its path, relative to the asset folder.
	struct AssetId
	{
		std::uint64_t value = 0;

		constexpr auto operator<=>(const AssetId&) const = default;
	};

	// "Textures\Hero.asset" and "textures/hero.asset" are the same file on Windows: they get the same id.
	inline std::string NormalizeAssetPath(std::string_view path)
	{
		std::string normalized;
		normalized.reserve(path.size());
		for (const char c : path)
			normalized += c == '\\' ? '/' : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

		while (normalized.starts_with("./"))
			normalized.erase(0, 2);

		return normalized;
	}

	inline AssetId AssetIdFromPath(std::string_view path)
	{
		return AssetId{Fnv1a64(NormalizeAssetPath(path))};
	}
}

// Lets AssetId be the key of a std::unordered_map.
template <>
struct std::hash<AssetEngine::AssetId>
{
	std::size_t operator()(AssetEngine::AssetId id) const noexcept
	{
		return static_cast<std::size_t>(id.value);
	}
};

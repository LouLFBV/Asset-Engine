#pragma once

#include <AssetEngine/Serialization/Archive.h>

#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>

namespace AssetEngine
{
	template <typename T>
	concept ArchivePrimitive =
		std::same_as<T, bool> || std::same_as<T, std::int32_t> || std::same_as<T, std::uint32_t> ||
		std::same_as<T, std::int64_t> || std::same_as<T, std::uint64_t> ||
		std::same_as<T, float> || std::same_as<T, double> || std::same_as<T, std::string>;

	template <ArchivePrimitive T>
	struct Serializer<T>
	{
		static void Process(Archive& archive, std::string_view name, T& value)
		{
			archive.Primitive(name, value);
		}
	};
}

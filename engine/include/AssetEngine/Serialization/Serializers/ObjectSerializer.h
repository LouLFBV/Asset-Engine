#pragma once

#include <AssetEngine/Serialization/Archive.h>

#include <string_view>

namespace AssetEngine
{
	template <typename T>
	concept HasSerialize = requires(T& value, Archive& archive)
	{
		value.Serialize(archive);
	};

	template <HasSerialize T>
	struct Serializer<T>
	{
		static void Process(Archive& archive, std::string_view name, T& value)
		{
			archive.BeginObject(name);
			value.Serialize(archive);
			archive.EndObject();
		}
	};
}

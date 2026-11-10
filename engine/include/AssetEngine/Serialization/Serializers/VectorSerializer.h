#pragma once

#include <AssetEngine/Serialization/Archive.h>

#include <cstddef>
#include <string_view>
#include <vector>

namespace AssetEngine
{
	template <Serializable T>
	struct Serializer<std::vector<T>>
	{
		static void Process(Archive& archive, std::string_view name, std::vector<T>& values)
		{
			std::size_t size = values.size();
			archive.BeginArray(name, size);

			if (archive.IsLoading())
				values.resize(size);

			for (T& value : values)
				archive.Value("", value);

			archive.EndArray();
		}
	};
}

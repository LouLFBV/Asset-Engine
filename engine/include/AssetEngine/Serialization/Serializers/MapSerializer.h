#pragma once

#include <AssetEngine/Serialization/Archive.h>

#include <cstddef>
#include <map>
#include <string_view>
#include <utility>

namespace AssetEngine
{
	template <Serializable K, Serializable V>
	struct Serializer<std::map<K, V>>
	{
		static void Process(Archive& archive, std::string_view name, std::map<K, V>& values)
		{
			std::size_t size = values.size();
			archive.BeginArray(name, size);

			if (archive.IsLoading())
				Load(archive, size, values);
			else
				Save(archive, values);

			archive.EndArray();
		}

	private:
		static void Load(Archive& archive, std::size_t size, std::map<K, V>& values)
		{
			values.clear();
			for (std::size_t i = 0; i < size && !archive.HasFailed(); ++i)
			{
				K key{};
				V value{};
				archive.BeginObject("");
				archive.Value("key", key);
				archive.Value("value", value);
				archive.EndObject();
				values.emplace(std::move(key), std::move(value));
			}
		}

		static void Save(Archive& archive, std::map<K, V>& values)
		{
			for (auto& [key, value] : values)
			{
				K keyCopy = key;
				archive.BeginObject("");
				archive.Value("key", keyCopy);
				archive.Value("value", value);
				archive.EndObject();
			}
		}
	};
}

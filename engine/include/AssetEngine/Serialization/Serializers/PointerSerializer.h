#pragma once

#include <AssetEngine/Core/Reflection/Factory.h>
#include <AssetEngine/Core/Reflection/Reflect.h>
#include <AssetEngine/Serialization/Archive.h>
#include <AssetEngine/Serialization/Serializers/ObjectSerializer.h>

#include <memory>
#include <string>
#include <string_view>

namespace AssetEngine
{
	// A pointer to a base class, which may hold any derived type.
	// T::Serialize must be virtual, so the real type writes and reads its own fields.
	template <typename T>
		requires Reflected<T> && HasSerialize<T>
	struct Serializer<std::unique_ptr<T>>
	{
		static void Process(Archive& archive, std::string_view name, std::unique_ptr<T>& pointer)
		{
			archive.BeginObject(name);

			if (archive.IsLoading())
				Load(archive, pointer);
			else
				Save(archive, pointer);

			archive.EndObject();
		}

	private:
		static void Save(Archive& archive, std::unique_ptr<T>& pointer)
		{
			// The real type is written first. An empty name means a null pointer.
			std::string typeName = pointer ? std::string(pointer->GetTypeInfo().GetName()) : std::string();
			archive.Value("type", typeName);

			if (pointer)
				archive.Value("data", *pointer);
		}

		static void Load(Archive& archive, std::unique_ptr<T>& pointer)
		{
			std::string typeName;
			archive.Value("type", typeName);
			if (archive.HasFailed())
				return;

			if (typeName.empty())
			{
				pointer.reset();
				return;
			}

			// The Factory creates the real type, then its own Serialize fills it.
			pointer = Factory::Create<T>(TypeIdFromName(typeName));
			if (!pointer)
			{
				archive.Fail(ArchiveError::InvalidData);
				return;
			}

			archive.Value("data", *pointer);
		}
	};
}

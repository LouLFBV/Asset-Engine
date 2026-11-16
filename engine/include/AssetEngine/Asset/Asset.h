#pragma once

#include <AssetEngine/Core/Reflection/Reflect.h>
#include <AssetEngine/Serialization/Archive.h>

namespace AssetEngine
{
	// The base of every asset type: a texture, a mesh, a config...
	// A derived type lists its bases in ASSET_REFLECT, overrides Serialize, and is registered with ASSET_REGISTER_TYPE.
	class Asset
	{
		ASSET_REFLECT(Asset, 1)

		virtual ~Asset() = default;

		// Writes or reads the fields of the asset. Virtual: the real type handles its own fields.
		virtual void Serialize(Archive& archive) = 0;
	};
}

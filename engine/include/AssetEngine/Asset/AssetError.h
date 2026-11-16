#pragma once

#include <string_view>

namespace AssetEngine
{
	enum class AssetError
	{
		None,
		InvalidFormat,    // Not an .asset file, or a header that does not match the content.
		IntegrityFailure, // The digest does not match: the file was corrupted or modified.
		UnknownType,      // The type of the asset is not registered.
		InvalidData       // The payload does not match the fields of the type.
	};

	constexpr std::string_view ToString(AssetError error)
	{
		switch (error)
		{
		case AssetError::None: return "None";
		case AssetError::InvalidFormat: return "InvalidFormat";
		case AssetError::IntegrityFailure: return "IntegrityFailure";
		case AssetError::UnknownType: return "UnknownType";
		case AssetError::InvalidData: return "InvalidData";
		}

		return "Unknown";
	}
}

#pragma once

#include <string_view>

namespace AssetEngine
{
	enum class AssetError
	{
		None,
		NotInRegistry,    // No asset has this id: the path is wrong, or the registry was not scanned.
		FileNotFound,     // The registry knows the asset, but its file cannot be opened.
		InvalidFormat,    // Not an .asset file, or a header that does not match the content.
		IntegrityFailure, // The digest does not match: the file was corrupted or modified.
		UnknownType,      // The type of the asset is not registered.
		InvalidData,      // The payload does not match the fields of the type.
		WrongType         // The asset exists, but is not of the type asked for.
	};

	constexpr std::string_view ToString(AssetError error)
	{
		switch (error)
		{
		case AssetError::None: return "None";
		case AssetError::NotInRegistry: return "NotInRegistry";
		case AssetError::FileNotFound: return "FileNotFound";
		case AssetError::InvalidFormat: return "InvalidFormat";
		case AssetError::IntegrityFailure: return "IntegrityFailure";
		case AssetError::UnknownType: return "UnknownType";
		case AssetError::InvalidData: return "InvalidData";
		case AssetError::WrongType: return "WrongType";
		}

		return "Unknown";
	}
}

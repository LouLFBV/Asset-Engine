#pragma once

#include <AssetEngine/Serialization/Json/JsonValue.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace AssetEngine
{
	struct JsonParseError
	{
		std::size_t position = 0;
		std::string message;
	};

	// Turns JSON text into a tree. Returns nothing, and fills error when it is given,
	// if the text is not valid JSON.
	std::optional<JsonValue> ParseJson(std::string_view text, JsonParseError* error = nullptr);
}

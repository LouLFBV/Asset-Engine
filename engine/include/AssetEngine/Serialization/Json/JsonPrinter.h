#pragma once

#include <AssetEngine/Serialization/Json/JsonValue.h>

#include <string>

namespace AssetEngine
{
	// Turns a JSON tree into readable text, indented with two spaces.
	std::string PrintJson(const JsonValue& value);
}

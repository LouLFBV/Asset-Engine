#pragma once

#include <AssetEngine/Core/IO/MemoryStream.h>
#include <AssetEngine/Serialization/Binary/BinaryReader.h>
#include <AssetEngine/Serialization/Binary/BinaryWriter.h>
#include <AssetEngine/Serialization/Serialization.h>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <initializer_list>
#include <vector>

namespace AssetEngine::Tests
{
	// Saves a value in binary, then loads it back into a fresh value.
	template <Serializable T>
	T BinaryRoundTrip(T value)
	{
		MemoryStream stream;
		BinaryWriter writer(stream);
		writer.Value("value", value);
		REQUIRE_FALSE(writer.HasFailed());

		REQUIRE(stream.Seek(0));

		T loaded{};
		BinaryReader reader(stream);
		reader.Value("value", loaded);
		REQUIRE_FALSE(reader.HasFailed());
		return loaded;
	}

	// Returns the bytes a value is saved as.
	template <Serializable T>
	std::vector<std::byte> BinaryBytes(T value)
	{
		MemoryStream stream;
		BinaryWriter writer(stream);
		writer.Value("value", value);
		REQUIRE_FALSE(writer.HasFailed());

		const auto data = stream.Data();
		return {data.begin(), data.end()};
	}

	// Builds a byte list from numbers, to write expected bytes easily.
	inline std::vector<std::byte> Bytes(std::initializer_list<int> values)
	{
		std::vector<std::byte> bytes;
		for (int value : values)
			bytes.push_back(static_cast<std::byte>(value));

		return bytes;
	}
}

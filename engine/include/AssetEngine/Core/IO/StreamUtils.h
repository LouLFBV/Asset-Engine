#pragma once

#include <AssetEngine/Core/IO/IInputStream.h>

#include <cstddef>
#include <vector>

namespace AssetEngine
{
	// Reads a whole stream from its start. Returns fewer bytes if the stream ends early.
	inline std::vector<std::byte> ReadAll(IInputStream& stream)
	{
		if (!stream.Seek(0))
			return {};

		std::vector<std::byte> bytes(stream.Size());
		bytes.resize(stream.Read(bytes));
		return bytes;
	}
}

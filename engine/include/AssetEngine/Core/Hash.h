#pragma once

#include <cstdint>
#include <string_view>

namespace AssetEngine
{
	// FNV-1a on 64 bits: a short, fast hash, usable at compile time.
	// it only turns names into identifiers.
	constexpr std::uint64_t Fnv1a64(std::string_view text)
	{
		std::uint64_t hash = 14695981039346656037ull;
		for (const char c : text)
		{
			hash ^= static_cast<std::uint8_t>(c);
			hash *= 1099511628211ull;
		}

		return hash;
	}
}

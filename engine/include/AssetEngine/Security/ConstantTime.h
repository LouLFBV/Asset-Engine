#pragma once

#include <cstddef>
#include <span>

namespace AssetEngine
{
	// Compares two byte sequences in a time that does not depend on where they differ.
	// A normal comparison stops at the first difference: by measuring how long it takes,
	// an attacker could guess a valid digest byte after byte.
	inline bool ConstantTimeEquals(std::span<const std::byte> a, std::span<const std::byte> b)
	{
		// The sizes are not secret: they are fixed by the algorithm.
		if (a.size() != b.size())
			return false;

		std::byte difference{0};
		for (std::size_t i = 0; i < a.size(); ++i)
			difference |= a[i] ^ b[i];

		return difference == std::byte{0};
	}
}

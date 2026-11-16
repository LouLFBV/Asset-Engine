#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace AssetEngine::Tests
{
	// The bytes of a text, without its terminating zero.
	inline std::span<const std::byte> AsBytes(std::string_view text)
	{
		return std::as_bytes(std::span<const char>(text.data(), text.size()));
	}

	inline std::vector<std::byte> Repeat(int value, std::size_t count)
	{
		return std::vector<std::byte>(count, static_cast<std::byte>(value));
	}

	// Writes bytes in lowercase hexadecimal, the way official test vectors are given.
	inline std::string ToHex(std::span<const std::byte> bytes)
	{
		constexpr std::string_view digits = "0123456789abcdef";

		std::string text;
		for (const std::byte b : bytes)
		{
			const auto value = std::to_integer<unsigned>(b);
			text += digits[value >> 4];
			text += digits[value & 0xFu];
		}

		return text;
	}
}

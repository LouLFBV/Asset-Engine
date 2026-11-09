#pragma once

#include <cstddef>
#include <span>

namespace AssetEngine
{
	class IInputStream
	{
	public:
		virtual ~IInputStream() = default;

		virtual std::size_t Read(std::span<std::byte> buffer) = 0;
		virtual std::size_t Size() const = 0;
		virtual bool Seek(std::size_t position) = 0;
	};
}

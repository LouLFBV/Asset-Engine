#pragma once

#include <cstddef>
#include <span>

namespace AssetEngine
{
	class IOutputStream
	{
	public:
		virtual ~IOutputStream() = default;

		virtual bool Write(std::span<const std::byte> data) = 0;
	};
}

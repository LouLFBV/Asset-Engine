#pragma once

#include <AssetEngine/Core/IO/IInputStream.h>
#include <AssetEngine/Serialization/Archive.h>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace AssetEngine
{
	class BinaryReader final : public Archive
	{
	public:
		static constexpr bool Loading = true;

		explicit BinaryReader(IInputStream& stream);

		bool IsLoading() const override { return Loading; }

		void Primitive(std::string_view name, bool& value) override;
		void Primitive(std::string_view name, std::int32_t& value) override;
		void Primitive(std::string_view name, std::uint32_t& value) override;
		void Primitive(std::string_view name, std::int64_t& value) override;
		void Primitive(std::string_view name, std::uint64_t& value) override;
		void Primitive(std::string_view name, float& value) override;
		void Primitive(std::string_view name, double& value) override;
		void Primitive(std::string_view name, std::string& value) override;

		void BeginObject(std::string_view name) override;
		void EndObject() override;
		void BeginArray(std::string_view name, std::size_t& size) override;
		void EndArray() override;

	private:
		bool ReadBytes(std::span<std::byte> bytes);
		bool ReadVarint(std::uint64_t& value);
		bool ReadSize(std::size_t& size);

		template <std::unsigned_integral T>
		bool ReadLittleEndian(T& value)
		{
			std::byte bytes[sizeof(T)];
			if (!ReadBytes(bytes))
				return false;

			value = 0;
			for (std::size_t i = 0; i < sizeof(T); ++i)
				value |= static_cast<T>(std::to_integer<T>(bytes[i]) << (8 * i));

			return true;
		}

		IInputStream& m_stream;
	};
}

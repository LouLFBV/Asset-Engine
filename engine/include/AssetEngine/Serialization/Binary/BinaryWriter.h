#pragma once

#include <AssetEngine/Core/IO/IOutputStream.h>
#include <AssetEngine/Serialization/Archive.h>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace AssetEngine
{
	class BinaryWriter final : public Archive
	{
	public:
		static constexpr bool Loading = false;

		explicit BinaryWriter(IOutputStream& stream);

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
		void WriteBytes(std::span<const std::byte> bytes);
		void WriteVarint(std::uint64_t value);

		template <std::unsigned_integral T>
		void WriteLittleEndian(T value)
		{
			std::byte bytes[sizeof(T)];
			for (std::size_t i = 0; i < sizeof(T); ++i)
				bytes[i] = static_cast<std::byte>((value >> (8 * i)) & 0xFF);

			WriteBytes(bytes);
		}

		IOutputStream& m_stream;
	};
}

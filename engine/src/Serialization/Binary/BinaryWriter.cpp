#include <AssetEngine/Serialization/Binary/BinaryWriter.h>

#include <bit>

namespace AssetEngine
{
	BinaryWriter::BinaryWriter(IOutputStream& stream)
		: m_stream(stream)
	{
	}

	void BinaryWriter::Primitive(std::string_view, bool& value)
	{
		const std::byte byte = value ? std::byte{1} : std::byte{0};
		WriteBytes(std::span<const std::byte>(&byte, 1));
	}

	void BinaryWriter::Primitive(std::string_view, std::int32_t& value)
	{
		WriteLittleEndian(static_cast<std::uint32_t>(value));
	}

	void BinaryWriter::Primitive(std::string_view, std::uint32_t& value)
	{
		WriteLittleEndian(value);
	}

	void BinaryWriter::Primitive(std::string_view, std::int64_t& value)
	{
		WriteLittleEndian(static_cast<std::uint64_t>(value));
	}

	void BinaryWriter::Primitive(std::string_view, std::uint64_t& value)
	{
		WriteLittleEndian(value);
	}

	void BinaryWriter::Primitive(std::string_view, float& value)
	{
		WriteLittleEndian(std::bit_cast<std::uint32_t>(value));
	}

	void BinaryWriter::Primitive(std::string_view, double& value)
	{
		WriteLittleEndian(std::bit_cast<std::uint64_t>(value));
	}

	void BinaryWriter::Primitive(std::string_view, std::string& value)
	{
		WriteVarint(value.size());
		WriteBytes(std::as_bytes(std::span(value.data(), value.size())));
	}

	void BinaryWriter::BeginObject(std::string_view)
	{
	}

	void BinaryWriter::EndObject()
	{
	}

	void BinaryWriter::BeginArray(std::string_view, std::size_t& size)
	{
		WriteVarint(size);
	}

	void BinaryWriter::EndArray()
	{
	}

	void BinaryWriter::WriteBytes(std::span<const std::byte> bytes)
	{
		if (HasFailed())
			return;

		if (!m_stream.Write(bytes))
			Fail(ArchiveError::StreamError);
	}

	void BinaryWriter::WriteVarint(std::uint64_t value)
	{
		std::byte bytes[10];
		std::size_t count = 0;

		do
		{
			auto part = static_cast<std::uint8_t>(value & 0x7F);
			value >>= 7;
			if (value != 0)
				part |= 0x80;

			bytes[count++] = static_cast<std::byte>(part);
		}
		while (value != 0);

		WriteBytes(std::span<const std::byte>(bytes, count));
	}
}

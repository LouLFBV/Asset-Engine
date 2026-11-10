#include <AssetEngine/Serialization/Binary/BinaryReader.h>

#include <bit>
#include <utility>

namespace AssetEngine
{
	BinaryReader::BinaryReader(IInputStream& stream)
		: m_stream(stream)
	{
	}

	void BinaryReader::Primitive(std::string_view, bool& value)
	{
		std::byte byte{};
		if (!ReadBytes(std::span<std::byte>(&byte, 1)))
			return;

		const auto raw = std::to_integer<int>(byte);
		if (raw > 1)
		{
			Fail(ArchiveError::InvalidData);
			return;
		}

		value = raw == 1;
	}

	void BinaryReader::Primitive(std::string_view, std::int32_t& value)
	{
		std::uint32_t raw = 0;
		if (ReadLittleEndian(raw))
			value = static_cast<std::int32_t>(raw);
	}

	void BinaryReader::Primitive(std::string_view, std::uint32_t& value)
	{
		ReadLittleEndian(value);
	}

	void BinaryReader::Primitive(std::string_view, std::int64_t& value)
	{
		std::uint64_t raw = 0;
		if (ReadLittleEndian(raw))
			value = static_cast<std::int64_t>(raw);
	}

	void BinaryReader::Primitive(std::string_view, std::uint64_t& value)
	{
		ReadLittleEndian(value);
	}

	void BinaryReader::Primitive(std::string_view, float& value)
	{
		std::uint32_t raw = 0;
		if (ReadLittleEndian(raw))
			value = std::bit_cast<float>(raw);
	}

	void BinaryReader::Primitive(std::string_view, double& value)
	{
		std::uint64_t raw = 0;
		if (ReadLittleEndian(raw))
			value = std::bit_cast<double>(raw);
	}

	void BinaryReader::Primitive(std::string_view, std::string& value)
	{
		std::size_t size = 0;
		if (!ReadSize(size))
			return;

		std::string text(size, '\0');
		if (!ReadBytes(std::as_writable_bytes(std::span(text.data(), text.size()))))
			return;

		value = std::move(text);
	}

	void BinaryReader::BeginObject(std::string_view)
	{
	}

	void BinaryReader::EndObject()
	{
	}

	void BinaryReader::BeginArray(std::string_view, std::size_t& size)
	{
		if (!ReadSize(size))
			size = 0;
	}

	void BinaryReader::EndArray()
	{
	}

	bool BinaryReader::ReadBytes(std::span<std::byte> bytes)
	{
		if (HasFailed())
			return false;

		if (m_stream.Read(bytes) != bytes.size())
		{
			Fail(ArchiveError::EndOfData);
			return false;
		}

		return true;
	}

	bool BinaryReader::ReadVarint(std::uint64_t& value)
	{
		value = 0;
		for (unsigned shift = 0; shift < 64; shift += 7)
		{
			std::byte byte{};
			if (!ReadBytes(std::span<std::byte>(&byte, 1)))
				return false;

			const auto bits = std::to_integer<std::uint64_t>(byte);
			value |= (bits & 0x7F) << shift;

			if ((bits & 0x80) == 0)
				return true;
		}

		// More than 10 bytes: this is not a valid 64-bit varint.
		Fail(ArchiveError::InvalidData);
		return false;
	}

	bool BinaryReader::ReadSize(std::size_t& size)
	{
		std::uint64_t value = 0;
		if (!ReadVarint(value))
			return false;

		// A size larger than the whole stream cannot be valid: refuse it instead of allocating it.
		if (value > m_stream.Size())
		{
			Fail(ArchiveError::InvalidData);
			return false;
		}

		size = static_cast<std::size_t>(value);
		return true;
	}
}

#include "BinaryTestUtils.h"

#include <AssetEngine/Core/IO/IOutputStream.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

using namespace AssetEngine;
using namespace AssetEngine::Tests;

namespace
{
	struct Hero
	{
		std::string name = "Lyra";
		std::vector<std::int32_t> scores{1, 2, 3};

		void Serialize(Archive& archive)
		{
			archive.Value("name", name);
			archive.Value("scores", scores);
		}
	};

	// Output stream that refuses every write, like a full disk.
	class FullDiskStream final : public IOutputStream
	{
	public:
		bool Write(std::span<const std::byte>) override { return false; }
	};
}

TEST_CASE("Binary reader reports missing data")
{
	MemoryStream stream;
	BinaryReader reader(stream);

	std::int32_t value = 42;
	reader.Value("value", value);

	REQUIRE(reader.GetError() == ArchiveError::EndOfData);
	REQUIRE(value == 42);
}

TEST_CASE("Binary reader reports a truncated file")
{
	auto bytes = BinaryBytes(Hero{});
	bytes.resize(bytes.size() / 2);

	MemoryStream stream(bytes);
	BinaryReader reader(stream);
	Hero loaded;
	reader.Value("hero", loaded);

	REQUIRE(reader.GetError() == ArchiveError::EndOfData);
}

TEST_CASE("Binary reader refuses an invalid boolean")
{
	MemoryStream stream(Bytes({0x02}));
	BinaryReader reader(stream);

	bool value = false;
	reader.Value("value", value);

	REQUIRE(reader.GetError() == ArchiveError::InvalidData);
}

TEST_CASE("Binary reader refuses a size larger than the data")
{
	// A varint worth 2^35, about 34 billion elements, in a 6-byte stream.
	MemoryStream stream(Bytes({0x80, 0x80, 0x80, 0x80, 0x80, 0x01}));
	BinaryReader reader(stream);

	std::vector<std::int32_t> values;
	reader.Value("values", values);

	REQUIRE(reader.GetError() == ArchiveError::InvalidData);
	REQUIRE(values.empty());
}

TEST_CASE("Binary reader refuses a varint that never ends")
{
	MemoryStream stream(std::vector<std::byte>(11, std::byte{0xFF}));
	BinaryReader reader(stream);

	std::string text;
	reader.Value("text", text);

	REQUIRE(reader.GetError() == ArchiveError::InvalidData);
}

TEST_CASE("Binary reader stops reading after the first error")
{
	// An invalid boolean, followed by a valid int32 worth 5.
	MemoryStream stream(Bytes({0x02, 0x05, 0x00, 0x00, 0x00}));
	BinaryReader reader(stream);

	bool flag = false;
	std::int32_t number = 42;
	reader.Value("flag", flag);
	reader.Value("number", number);

	REQUIRE(reader.GetError() == ArchiveError::InvalidData);
	REQUIRE(number == 42);
}

TEST_CASE("Binary writer reports a stream that refuses to write")
{
	FullDiskStream stream;
	BinaryWriter writer(stream);

	std::int32_t value = 1;
	writer.Value("value", value);

	REQUIRE(writer.GetError() == ArchiveError::StreamError);
}

#include <AssetEngine/Core/IO/MemoryStream.h>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <vector>

using namespace AssetEngine;

TEST_CASE("MemoryStream reads back what was written")
{
	MemoryStream stream;
	const std::array<std::byte, 3> input{std::byte{1}, std::byte{2}, std::byte{3}};

	REQUIRE(stream.Write(input));
	REQUIRE(stream.Seek(0));

	std::array<std::byte, 3> output{};
	REQUIRE(stream.Read(output) == 3);
	REQUIRE(output == input);
}

TEST_CASE("MemoryStream reads less at the end of the data")
{
	MemoryStream stream;
	const std::array<std::byte, 2> input{std::byte{1}, std::byte{2}};
	REQUIRE(stream.Write(input));
	REQUIRE(stream.Seek(1));

	std::array<std::byte, 8> output{};
	REQUIRE(stream.Read(output) == 1);
	REQUIRE(output[0] == std::byte{2});
	REQUIRE(stream.Read(output) == 0);
}

TEST_CASE("MemoryStream reports its size and checks Seek")
{
	MemoryStream stream;
	const std::array<std::byte, 4> input{};
	REQUIRE(stream.Write(input));

	REQUIRE(stream.Size() == 4);
	REQUIRE(stream.Seek(4));
	REQUIRE_FALSE(stream.Seek(5));
}

TEST_CASE("MemoryStream reads the data given to its constructor")
{
	MemoryStream stream(std::vector<std::byte>{std::byte{4}, std::byte{5}});
	REQUIRE(stream.Size() == 2);
	REQUIRE(stream.Data()[1] == std::byte{5});

	std::array<std::byte, 2> output{};
	REQUIRE(stream.Read(output) == 2);
	REQUIRE(output[0] == std::byte{4});
}

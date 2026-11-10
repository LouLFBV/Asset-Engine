#include <AssetEngine/Core/IO/FileInputStream.h>
#include <AssetEngine/Core/IO/FileOutputStream.h>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <filesystem>
#include <type_traits>

using namespace AssetEngine;

namespace
{
	static_assert(std::is_base_of_v<IInputStream, FileInputStream>);
	static_assert(!std::is_base_of_v<IOutputStream, FileInputStream>);
	static_assert(std::is_base_of_v<IOutputStream, FileOutputStream>);
	static_assert(!std::is_base_of_v<IInputStream, FileOutputStream>);

	std::filesystem::path TestFile(const char* name)
	{
		return std::filesystem::temp_directory_path() / name;
	}
}

TEST_CASE("File streams read back what was written")
{
	const auto path = TestFile("assetengine_file_stream_test.bin");
	const std::array<std::byte, 3> input{std::byte{1}, std::byte{2}, std::byte{3}};

	{
		FileOutputStream output(path);
		REQUIRE(output.IsOpen());
		REQUIRE(output.Write(input));
	}

	{
		FileInputStream file(path);
		REQUIRE(file.IsOpen());
		REQUIRE(file.Size() == 3);

		std::array<std::byte, 8> buffer{};
		REQUIRE(file.Read(buffer) == 3);
		REQUIRE(buffer[2] == std::byte{3});
		REQUIRE(file.Read(buffer) == 0);

		REQUIRE(file.Seek(1));
		REQUIRE(file.Read(buffer) == 2);
		REQUIRE(buffer[0] == std::byte{2});
		REQUIRE_FALSE(file.Seek(4));
	}

	std::filesystem::remove(path);
}

TEST_CASE("FileInputStream reports a missing file")
{
	const auto path = TestFile("assetengine_missing_file.bin");
	std::filesystem::remove(path);

	FileInputStream file(path);
	REQUIRE_FALSE(file.IsOpen());

	std::array<std::byte, 4> buffer{};
	REQUIRE(file.Read(buffer) == 0);
}

TEST_CASE("FileOutputStream replaces an existing file")
{
	const auto path = TestFile("assetengine_truncate_test.bin");
	const std::array<std::byte, 3> longInput{std::byte{1}, std::byte{2}, std::byte{3}};
	const std::array<std::byte, 1> shortInput{std::byte{9}};

	{
		FileOutputStream output(path);
		REQUIRE(output.Write(longInput));
	}

	{
		FileOutputStream output(path);
		REQUIRE(output.Write(shortInput));
	}

	{
		FileInputStream file(path);
		REQUIRE(file.Size() == 1);
	}

	std::filesystem::remove(path);
}

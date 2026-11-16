#pragma once

#include <AssetEngine/Asset/AssetFile.h>
#include <AssetEngine/Core/IO/FileOutputStream.h>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <ios>
#include <random>
#include <string>
#include <system_error>
#include <thread>

namespace AssetEngine::Tests
{
	// A folder of its own for one test, deleted at the end of the test.
	class TempDirectory
	{
	public:
		TempDirectory()
			: m_path(std::filesystem::temp_directory_path() / ("AssetEngineTests-" + std::to_string(std::random_device{}())))
		{
			std::filesystem::create_directories(m_path);
		}

		~TempDirectory()
		{
			std::error_code error;
			std::filesystem::remove_all(m_path, error);
		}

		TempDirectory(const TempDirectory&) = delete;
		TempDirectory& operator=(const TempDirectory&) = delete;

		const std::filesystem::path& Path() const { return m_path; }

	private:
		std::filesystem::path m_path;
	};

	// An antivirus may lock a file for a moment just after it was written:
	// a change that fails is tried again for up to half a second, instead of being silently skipped.
	template <typename Change>
	bool Retry(Change change)
	{
		for (int attempt = 0; attempt < 50; ++attempt)
		{
			if (change())
				return true;

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		return false;
	}

	inline void RemoveFile(const std::filesystem::path& file)
	{
		REQUIRE(Retry([&] {
			std::error_code error;
			return std::filesystem::remove(file, error) && !error;
		}));
	}

	// Changes one bit of a file, the smallest possible corruption.
	inline void FlipBit(const std::filesystem::path& file, std::streamoff position)
	{
		REQUIRE(Retry([&] {
			std::fstream stream(file, std::ios::in | std::ios::out | std::ios::binary);
			stream.seekg(position);
			const int original = stream.get();
			if (!stream || original == std::char_traits<char>::eof())
				return false;

			stream.seekp(position);
			stream.put(static_cast<char>(original ^ 0x01));
			return static_cast<bool>(stream.flush());
		}));
	}

	// Writes an .asset file, and creates its folders if needed.
	inline void WriteAssetFile(const std::filesystem::path& file, Asset& asset, const AssetMetadata& metadata = {},
		AssetEncoding encoding = AssetEncoding::Binary, const SecurityPolicy& policy = SecurityPolicy::Checksum())
	{
		std::filesystem::create_directories(file.parent_path());

		FileOutputStream stream(file);
		REQUIRE(stream.IsOpen());
		REQUIRE(WriteAsset(stream, asset, metadata, encoding, policy));
	}
}

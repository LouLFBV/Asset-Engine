#pragma once

#include <AssetEngine/Asset/AssetFile.h>
#include <AssetEngine/Core/IO/FileOutputStream.h>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <random>
#include <string>
#include <system_error>

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

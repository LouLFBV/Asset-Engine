#pragma once

#include <AssetEngine/Asset/AssetFile.h>
#include <AssetEngine/Asset/AssetId.h>
#include <AssetEngine/Asset/AssetMetadata.h>
#include <AssetEngine/Core/Reflection/TypeId.h>

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace AssetEngine
{
	// What the registry knows about one .asset file, without loading it.
	struct AssetRecord
	{
		AssetId id;
		std::string path; // Relative to the asset folder, with '/'.
		TypeId type;
		AssetEncoding encoding = AssetEncoding::Binary;
		AssetMetadata metadata;

		void Serialize(Archive& archive);
	};

	struct ScanReport
	{
		std::size_t found = 0;
		std::vector<std::string> errors;
	};

	// The list of every asset of a folder, built from the headers and metadata of the .asset files.
	class AssetRegistry
	{
	public:
		explicit AssetRegistry(std::filesystem::path root);

		const std::filesystem::path& GetRoot() const { return m_root; }

		// Reads every .asset file under the root folder, and replaces the previous list.
		ScanReport Scan();

		const AssetRecord* Find(AssetId id) const;
		const AssetRecord* Find(std::string_view path) const { return Find(AssetIdFromPath(path)); }

		// The results are sorted by path.
		std::vector<const AssetRecord*> GetAll() const;
		std::vector<const AssetRecord*> FindByType(TypeId type) const;
		std::vector<const AssetRecord*> FindByTag(std::string_view tag) const;

		std::filesystem::path GetFullPath(const AssetRecord& record) const { return m_root / record.path; }
		std::size_t Count() const { return m_records.size(); }

		// registry.bin: a cache of the list, protected by a CRC32, to start without scanning.
		// The .asset files stay the reference: if LoadCache fails, call Scan.
		bool SaveCache(const std::filesystem::path& file) const;
		bool LoadCache(const std::filesystem::path& file);

	private:
		std::filesystem::path m_root;
		std::unordered_map<AssetId, AssetRecord> m_records;
	};
}

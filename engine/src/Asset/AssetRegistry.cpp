#include <AssetEngine/Asset/AssetRegistry.h>

#include <AssetEngine/Core/IO/FileInputStream.h>
#include <AssetEngine/Core/IO/FileOutputStream.h>
#include <AssetEngine/Core/IO/MemoryStream.h>
#include <AssetEngine/Core/IO/StreamUtils.h>
#include <AssetEngine/Security/SecurityPolicy.h>
#include <AssetEngine/Serialization/Binary/BinaryReader.h>
#include <AssetEngine/Serialization/Binary/BinaryWriter.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <system_error>
#include <utility>

namespace AssetEngine
{
	void AssetRecord::Serialize(Archive& archive)
	{
		auto encodingValue = static_cast<std::uint32_t>(encoding);

		archive.Value("id", id.value);
		archive.Value("path", path);
		archive.Value("type", type.value);
		archive.Value("encoding", encodingValue);
		archive.Value("metadata", metadata);

		if (archive.IsLoading())
			encoding = static_cast<AssetEncoding>(encodingValue);
	}

	namespace
	{
		// Raised when the content of registry.bin changes, so that old caches are refused.
		constexpr std::uint32_t CacheVersion = 1;

		std::vector<const AssetRecord*> SortedByPath(std::vector<const AssetRecord*> records)
		{
			std::ranges::sort(records, {}, &AssetRecord::path);
			return records;
		}
	}

	AssetRegistry::AssetRegistry(std::filesystem::path root)
		: m_root(std::move(root))
	{
	}

	ScanReport AssetRegistry::Scan()
	{
		namespace fs = std::filesystem;

		ScanReport report;
		m_records.clear();

		std::error_code error;
		for (auto entry = fs::recursive_directory_iterator(m_root, error); !error && entry != fs::recursive_directory_iterator(); entry.increment(error))
		{
			if (!entry->is_regular_file() || entry->path().extension() != ".asset")
				continue;

			const std::string path = entry->path().lexically_relative(m_root).generic_string();

			// Only the header and the metadata are read: scanning stays fast, whatever the size of the assets.
			FileInputStream stream(entry->path());
			const std::optional<AssetInfo> info = stream.IsOpen() ? ReadAssetInfo(stream) : std::nullopt;
			if (!info)
			{
				report.errors.push_back(path + ": not a valid .asset file");
				continue;
			}

			const AssetId id = AssetIdFromPath(path);
			const auto [existing, inserted] = m_records.try_emplace(id, AssetRecord{id, path, info->header.type, info->header.encoding, info->metadata});
			if (!inserted)
				report.errors.push_back(path + ": same id as " + existing->second.path);
		}

		if (error)
			report.errors.push_back("cannot read the folder " + m_root.generic_string());

		report.found = m_records.size();
		return report;
	}

	const AssetRecord* AssetRegistry::Find(AssetId id) const
	{
		const auto entry = m_records.find(id);
		return entry != m_records.end() ? &entry->second : nullptr;
	}

	std::vector<const AssetRecord*> AssetRegistry::GetAll() const
	{
		std::vector<const AssetRecord*> records;
		for (const auto& [id, record] : m_records)
			records.push_back(&record);

		return SortedByPath(std::move(records));
	}

	std::vector<const AssetRecord*> AssetRegistry::FindByType(TypeId type) const
	{
		std::vector<const AssetRecord*> records;
		for (const auto& [id, record] : m_records)
		{
			if (record.type == type)
				records.push_back(&record);
		}

		return SortedByPath(std::move(records));
	}

	std::vector<const AssetRecord*> AssetRegistry::FindByTag(std::string_view tag) const
	{
		std::vector<const AssetRecord*> records;
		for (const auto& [id, record] : m_records)
		{
			if (std::ranges::find(record.metadata.tags, tag) != record.metadata.tags.end())
				records.push_back(&record);
		}

		return SortedByPath(std::move(records));
	}

	bool AssetRegistry::SaveCache(const std::filesystem::path& file) const
	{
		std::vector<AssetRecord> records;
		for (const AssetRecord* record : GetAll())
			records.push_back(*record);

		std::uint32_t version = CacheVersion;
		MemoryStream content;
		BinaryWriter writer(content);
		writer.Value("version", version);
		writer.Value("records", records);
		if (writer.HasFailed())
			return false;

		FileOutputStream stream(file);
		return stream.IsOpen() && stream.Write(SecurityPolicy::Checksum().Seal(content.Data()));
	}

	bool AssetRegistry::LoadCache(const std::filesystem::path& file)
	{
		FileInputStream stream(file);
		if (!stream.IsOpen())
			return false;

		const std::vector<std::byte> bytes = ReadAll(stream);
		const auto content = SecurityPolicy::Checksum().Open(bytes);
		if (!content)
			return false;

		MemoryStream contentStream({content->begin(), content->end()});
		BinaryReader reader(contentStream);

		std::uint32_t version = 0;
		std::vector<AssetRecord> records;
		reader.Value("version", version);
		reader.Value("records", records);
		if (reader.HasFailed() || version != CacheVersion)
			return false;

		// The current list is only replaced once the whole cache is known to be valid.
		std::unordered_map<AssetId, AssetRecord> loaded;
		for (AssetRecord& record : records)
		{
			const AssetId id = record.id;
			if (id != AssetIdFromPath(record.path) || !loaded.try_emplace(id, std::move(record)).second)
				return false;
		}

		m_records = std::move(loaded);
		return true;
	}
}

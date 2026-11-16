#pragma once

#include <AssetEngine/Asset/Asset.h>
#include <AssetEngine/Asset/AssetError.h>
#include <AssetEngine/Asset/AssetMetadata.h>
#include <AssetEngine/Core/IO/IInputStream.h>
#include <AssetEngine/Core/IO/IOutputStream.h>
#include <AssetEngine/Core/Reflection/TypeId.h>
#include <AssetEngine/Security/SecurityPolicy.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

// The .asset format:
//
//   [ header, 36 bytes ][ metadata ][ payload ][ digest ]
//
// - header and metadata are always binary: the registry reads them quickly, without the payload;
// - the payload holds the fields of the asset, in binary or in JSON;
// - the digest covers everything before it. Its algorithm comes from the SecurityPolicy, not from the file.

namespace AssetEngine
{
	enum class AssetEncoding : std::uint32_t
	{
		Binary = 0,
		Json = 1
	};

	struct AssetHeader
	{
		static constexpr std::uint32_t Magic = 0x474E4541; // "AENG" once written in little-endian.
		static constexpr std::uint32_t CurrentFormat = 1;
		static constexpr std::size_t Size = 36;

		std::uint32_t magic = Magic;
		std::uint32_t formatVersion = CurrentFormat;
		AssetEncoding encoding = AssetEncoding::Binary;
		TypeId type;
		std::uint64_t metadataSize = 0;
		std::uint64_t payloadSize = 0;

		void Serialize(Archive& archive);
	};

	struct AssetInfo
	{
		AssetHeader header;
		AssetMetadata metadata;
	};

	struct AssetLoadResult
	{
		std::unique_ptr<Asset> asset;
		AssetInfo info;
		AssetError error = AssetError::None;
	};

	// Writes an asset in the .asset format. Returns false if the asset or the stream fails.
	bool WriteAsset(IOutputStream& stream, Asset& asset, const AssetMetadata& metadata, AssetEncoding encoding, const SecurityPolicy& policy);

	// Reads the header and the metadata only: the payload is neither read nor verified.
	// Fast enough to scan a whole folder. Never use it to trust the content.
	std::optional<AssetInfo> ReadAssetInfo(IInputStream& stream);

	// Reads a whole asset: verifies the digest first, then creates the real type with the Factory.
	AssetLoadResult ReadAsset(IInputStream& stream, const SecurityPolicy& policy);
}

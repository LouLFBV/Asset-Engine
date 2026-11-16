#include <AssetEngine/Asset/AssetFile.h>

#include <AssetEngine/Core/IO/MemoryStream.h>
#include <AssetEngine/Core/IO/StreamUtils.h>
#include <AssetEngine/Core/Reflection/Factory.h>
#include <AssetEngine/Serialization/Binary/BinaryReader.h>
#include <AssetEngine/Serialization/Binary/BinaryWriter.h>
#include <AssetEngine/Serialization/Json/JsonReader.h>
#include <AssetEngine/Serialization/Json/JsonWriter.h>

#include <array>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

namespace AssetEngine
{
	void AssetHeader::Serialize(Archive& archive)
	{
		// The enum goes through a number, the only form an archive knows.
		auto encodingValue = static_cast<std::uint32_t>(encoding);

		archive.Value("magic", magic);
		archive.Value("formatVersion", formatVersion);
		archive.Value("encoding", encodingValue);
		archive.Value("type", type.value);
		archive.Value("metadataSize", metadataSize);
		archive.Value("payloadSize", payloadSize);

		if (archive.IsLoading())
			encoding = static_cast<AssetEncoding>(encodingValue);
	}

	namespace
	{
		constexpr std::string_view PayloadName = "payload";

		std::optional<AssetHeader> ParseHeader(std::span<const std::byte> bytes)
		{
			MemoryStream stream({bytes.begin(), bytes.end()});
			BinaryReader reader(stream);

			AssetHeader header;
			reader.Value("header", header);
			if (reader.HasFailed() || header.magic != AssetHeader::Magic || header.formatVersion != AssetHeader::CurrentFormat)
				return std::nullopt;

			if (header.encoding != AssetEncoding::Binary && header.encoding != AssetEncoding::Json)
				return std::nullopt;

			return header;
		}

		std::optional<AssetMetadata> ParseMetadata(std::span<const std::byte> bytes)
		{
			MemoryStream stream({bytes.begin(), bytes.end()});
			BinaryReader reader(stream);

			AssetMetadata metadata;
			reader.Value("metadata", metadata);
			if (reader.HasFailed())
				return std::nullopt;

			return metadata;
		}

		// Writes the fields of the asset with the archive of the encoding.
		bool WritePayload(MemoryStream& stream, Asset& asset, AssetEncoding encoding)
		{
			if (encoding == AssetEncoding::Json)
			{
				JsonWriter writer(stream);
				writer.Value(PayloadName, asset);
				return writer.Finish();
			}

			BinaryWriter writer(stream);
			writer.Value(PayloadName, asset);
			return !writer.HasFailed();
		}

		bool ReadPayload(std::span<const std::byte> bytes, Asset& asset, AssetEncoding encoding)
		{
			MemoryStream stream({bytes.begin(), bytes.end()});

			if (encoding == AssetEncoding::Json)
			{
				JsonReader reader(stream);
				reader.Value(PayloadName, asset);
				return !reader.HasFailed();
			}

			BinaryReader reader(stream);
			reader.Value(PayloadName, asset);
			return !reader.HasFailed();
		}
	}

	bool WriteAsset(IOutputStream& stream, Asset& asset, const AssetMetadata& metadata, AssetEncoding encoding, const SecurityPolicy& policy)
	{
		MemoryStream metadataStream;
		{
			AssetMetadata copy = metadata;
			BinaryWriter writer(metadataStream);
			writer.Value("metadata", copy);
			if (writer.HasFailed())
				return false;
		}

		MemoryStream payloadStream;
		if (!WritePayload(payloadStream, asset, encoding))
			return false;

		AssetHeader header;
		header.encoding = encoding;
		header.type = asset.GetTypeInfo().GetId();
		header.metadataSize = metadataStream.Size();
		header.payloadSize = payloadStream.Size();

		// The three blocks are put together in memory, then sealed with their digest.
		MemoryStream content;
		BinaryWriter headerWriter(content);
		headerWriter.Value("header", header);
		if (headerWriter.HasFailed())
			return false;

		content.Write(metadataStream.Data());
		content.Write(payloadStream.Data());

		return stream.Write(policy.Seal(content.Data()));
	}

	std::optional<AssetInfo> ReadAssetInfo(IInputStream& stream)
	{
		if (!stream.Seek(0))
			return std::nullopt;

		std::array<std::byte, AssetHeader::Size> headerBytes{};
		if (stream.Read(headerBytes) != headerBytes.size())
			return std::nullopt;

		std::optional<AssetHeader> header = ParseHeader(headerBytes);
		if (!header)
			return std::nullopt;

		// The size comes from the file: it is checked before allocating anything.
		if (header->metadataSize > stream.Size() - AssetHeader::Size)
			return std::nullopt;

		std::vector<std::byte> metadataBytes(static_cast<std::size_t>(header->metadataSize));
		if (stream.Read(metadataBytes) != metadataBytes.size())
			return std::nullopt;

		std::optional<AssetMetadata> metadata = ParseMetadata(metadataBytes);
		if (!metadata)
			return std::nullopt;

		return AssetInfo{*header, std::move(*metadata)};
	}

	AssetLoadResult ReadAsset(IInputStream& stream, const SecurityPolicy& policy)
	{
		AssetLoadResult result;
		const std::vector<std::byte> bytes = ReadAll(stream);

		// 1. The digest is verified before anything else is read.
		const std::optional<std::span<const std::byte>> content = policy.Open(bytes);
		if (!content)
		{
			result.error = AssetError::IntegrityFailure;
			return result;
		}

		// 2. The header must describe exactly the content.
		std::optional<AssetHeader> header;
		if (content->size() >= AssetHeader::Size)
			header = ParseHeader(content->first(AssetHeader::Size));

		const std::size_t blocksSize = content->size() - AssetHeader::Size;
		if (!header || header->metadataSize > blocksSize || header->payloadSize != blocksSize - header->metadataSize)
		{
			result.error = AssetError::InvalidFormat;
			return result;
		}

		const auto metadataBytes = content->subspan(AssetHeader::Size, static_cast<std::size_t>(header->metadataSize));
		const auto payloadBytes = content->subspan(AssetHeader::Size + metadataBytes.size());

		std::optional<AssetMetadata> metadata = ParseMetadata(metadataBytes);
		if (!metadata)
		{
			result.error = AssetError::InvalidFormat;
			return result;
		}

		// 3. The Factory creates the real type, then its own Serialize reads its fields.
		std::unique_ptr<Asset> asset = Factory::Create<Asset>(header->type);
		if (!asset)
		{
			result.error = AssetError::UnknownType;
			return result;
		}

		if (!ReadPayload(payloadBytes, *asset, header->encoding))
		{
			result.error = AssetError::InvalidData;
			return result;
		}

		result.asset = std::move(asset);
		result.info = AssetInfo{*header, std::move(*metadata)};
		return result;
	}
}

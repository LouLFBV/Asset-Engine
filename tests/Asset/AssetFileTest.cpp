#include <AssetEngine/Asset/AssetFile.h>
#include <AssetEngine/Core/IO/MemoryStream.h>
#include <AssetEngine/Core/Reflection/TypeRegistry.h>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

using namespace AssetEngine;

namespace
{
	class Texture : public Asset
	{
		ASSET_REFLECT(Texture, 1, Asset)

		void Serialize(Archive& archive) override
		{
			archive.Value("width", width);
			archive.Value("height", height);
			archive.Value("pixels", pixels);
		}

		std::int32_t width = 0;
		std::int32_t height = 0;
		std::vector<std::uint32_t> pixels;
	};

	// Never registered: the Factory cannot create it.
	class Note : public Asset
	{
		ASSET_REFLECT(Note, 1, Asset)

		void Serialize(Archive& archive) override { archive.Value("text", text); }

		std::string text = "hello";
	};

	// Counts the bytes read, to prove what is read and what is not.
	class CountingStream final : public IInputStream
	{
	public:
		explicit CountingStream(IInputStream& inner) : m_inner(inner) {}

		std::size_t Read(std::span<std::byte> buffer) override
		{
			const std::size_t count = m_inner.Read(buffer);
			bytesRead += count;
			return count;
		}

		std::size_t Size() const override { return m_inner.Size(); }
		bool Seek(std::size_t position) override { return m_inner.Seek(position); }

		std::size_t bytesRead = 0;

	private:
		IInputStream& m_inner;
	};

	Texture MakeTexture(std::size_t pixelCount = 4)
	{
		Texture texture;
		texture.width = 2;
		texture.height = static_cast<std::int32_t>(pixelCount / 2);
		for (std::size_t i = 0; i < pixelCount; ++i)
			texture.pixels.push_back(0xFF000000u + static_cast<std::uint32_t>(i));

		return texture;
	}

	AssetMetadata MakeMetadata()
	{
		return {3, {"textures/palette.asset"}, {"ui", "small"}};
	}

	SecurityPolicy SignedPolicy()
	{
		return SecurityPolicy::Signed({std::byte{1}, std::byte{2}, std::byte{3}});
	}

	std::vector<std::byte> Write(Asset& asset, AssetEncoding encoding, const SecurityPolicy& policy)
	{
		MemoryStream stream;
		REQUIRE(WriteAsset(stream, asset, MakeMetadata(), encoding, policy));

		const auto data = stream.Data();
		return {data.begin(), data.end()};
	}

	AssetLoadResult Read(std::vector<std::byte> bytes, const SecurityPolicy& policy)
	{
		MemoryStream stream(std::move(bytes));
		return ReadAsset(stream, policy);
	}

	bool Contains(std::span<const std::byte> bytes, std::string_view text)
	{
		const auto pattern = std::as_bytes(std::span<const char>(text.data(), text.size()));
		return !std::ranges::search(bytes, pattern).empty();
	}

	// Rewrites the content of a file sealed with a CRC32, the way an attacker without the HMAC key could.
	template <typename Change>
	std::vector<std::byte> Reseal(const std::vector<std::byte>& file, Change change)
	{
		const SecurityPolicy policy = SecurityPolicy::Checksum();
		const auto content = policy.Open(file);
		REQUIRE(content.has_value());

		std::vector<std::byte> changed(content->begin(), content->end());
		change(changed);
		return policy.Seal(changed);
	}
}

ASSET_REGISTER_TYPE(Texture)

TEST_CASE("An asset round-trips through the .asset format")
{
	for (const AssetEncoding encoding : {AssetEncoding::Binary, AssetEncoding::Json})
	{
		for (const SecurityPolicy& policy : {SecurityPolicy::Checksum(), SignedPolicy()})
		{
			Texture original = MakeTexture();
			const AssetLoadResult result = Read(Write(original, encoding, policy), policy);

			REQUIRE(result.error == AssetError::None);
			REQUIRE(result.asset != nullptr);
			REQUIRE(result.asset->GetTypeInfo().GetName() == "Texture");

			const auto* texture = dynamic_cast<const Texture*>(result.asset.get());
			REQUIRE(texture != nullptr);
			REQUIRE(texture->width == original.width);
			REQUIRE(texture->height == original.height);
			REQUIRE(texture->pixels == original.pixels);

			REQUIRE(result.info.header.type == TypeIdOf<Texture>);
			REQUIRE(result.info.header.encoding == encoding);
			REQUIRE(result.info.metadata == MakeMetadata());
		}
	}
}

TEST_CASE("An .asset file is a header, the metadata, the payload and the digest")
{
	Texture texture = MakeTexture();
	const SecurityPolicy policy = SignedPolicy();
	const auto file = Write(texture, AssetEncoding::Binary, policy);

	// The magic number, readable at the start of the file.
	REQUIRE(Contains(std::span(file).first(4), "AENG"));

	MemoryStream stream(file);
	const auto info = ReadAssetInfo(stream);
	REQUIRE(info.has_value());
	REQUIRE(file.size() == AssetHeader::Size + info->header.metadataSize + info->header.payloadSize + policy.GetDigestSize());
}

TEST_CASE("The metadata is read without the payload")
{
	// A big texture: 40 000 bytes of pixels.
	Texture texture = MakeTexture(10'000);
	MemoryStream file(Write(texture, AssetEncoding::Binary, SecurityPolicy::Checksum()));

	CountingStream stream(file);
	const auto info = ReadAssetInfo(stream);

	REQUIRE(info.has_value());
	REQUIRE(info->header.type == TypeIdOf<Texture>);
	REQUIRE(info->metadata == MakeMetadata());

	// Only the header and the metadata were read.
	REQUIRE(stream.bytesRead == AssetHeader::Size + info->header.metadataSize);
	REQUIRE(stream.bytesRead < 100);
}

TEST_CASE("A JSON payload stays readable")
{
	Texture texture = MakeTexture();
	const auto file = Write(texture, AssetEncoding::Json, SecurityPolicy::Checksum());

	REQUIRE(Contains(file, "\"width\": 2"));
}

TEST_CASE("Invalid asset files are refused")
{
	Texture texture = MakeTexture();
	const auto file = Write(texture, AssetEncoding::Json, SecurityPolicy::Checksum());

	SECTION("A modified byte")
	{
		auto changed = file;
		changed[AssetHeader::Size + 2] ^= std::byte{0x01};
		REQUIRE(Read(changed, SecurityPolicy::Checksum()).error == AssetError::IntegrityFailure);
	}

	SECTION("A file sealed with another policy")
	{
		REQUIRE(Read(file, SignedPolicy()).error == AssetError::IntegrityFailure);
	}

	SECTION("An empty file")
	{
		REQUIRE(Read({}, SecurityPolicy::Checksum()).error == AssetError::IntegrityFailure);
	}

	SECTION("A wrong magic number")
	{
		const auto changed = Reseal(file, [](std::vector<std::byte>& content) { content[0] = std::byte{'X'}; });
		REQUIRE(Read(changed, SecurityPolicy::Checksum()).error == AssetError::InvalidFormat);
	}

	SECTION("A header that does not match the content")
	{
		const auto changed = Reseal(file, [](std::vector<std::byte>& content) { content.push_back(std::byte{0}); });
		REQUIRE(Read(changed, SecurityPolicy::Checksum()).error == AssetError::InvalidFormat);
	}

	SECTION("A payload that does not match the type")
	{
		const auto changed = Reseal(file, [](std::vector<std::byte>& content) {
			const std::string_view field = "\"width\"";
			const auto pattern = std::as_bytes(std::span<const char>(field.data(), field.size()));
			auto found = std::ranges::search(content, pattern);
			REQUIRE_FALSE(found.empty());
			found[1] = std::byte{'W'};
		});

		REQUIRE(Read(changed, SecurityPolicy::Checksum()).error == AssetError::InvalidData);
	}

	SECTION("A type that is not registered")
	{
		Note note;
		REQUIRE(Read(Write(note, AssetEncoding::Binary, SecurityPolicy::Checksum()), SecurityPolicy::Checksum()).error == AssetError::UnknownType);
	}
}

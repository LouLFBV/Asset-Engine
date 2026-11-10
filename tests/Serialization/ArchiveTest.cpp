#include <AssetEngine/Serialization/Serialization.h>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

using namespace AssetEngine;

namespace
{
	struct Hero
	{
		std::string name = "Lyra";
		std::int32_t hp = 100;
		std::vector<std::vector<std::int32_t>> grid{{1, 2}, {3}};

		void Serialize(Archive& archive)
		{
			archive.Value("name", name);
			archive.Value("hp", hp);
			archive.Value("grid", grid);
		}
	};

	struct NotSerializable
	{
	};

	static_assert(Serializable<std::int32_t>);
	static_assert(Serializable<Hero>);
	static_assert(Serializable<std::vector<std::vector<Hero>>>);
	static_assert(Serializable<std::map<std::string, Hero>>);
	static_assert(!Serializable<NotSerializable>);
	static_assert(!Serializable<std::vector<NotSerializable>>);

	// Fake archive that only counts the calls it receives.
	class CountingArchive final : public Archive
	{
	public:
		int primitives = 0;
		int objects = 0;
		int arrays = 0;

		bool IsLoading() const override { return false; }

		void Primitive(std::string_view, bool&) override { ++primitives; }
		void Primitive(std::string_view, std::int32_t&) override { ++primitives; }
		void Primitive(std::string_view, std::uint32_t&) override { ++primitives; }
		void Primitive(std::string_view, std::int64_t&) override { ++primitives; }
		void Primitive(std::string_view, std::uint64_t&) override { ++primitives; }
		void Primitive(std::string_view, float&) override { ++primitives; }
		void Primitive(std::string_view, double&) override { ++primitives; }
		void Primitive(std::string_view, std::string&) override { ++primitives; }

		void BeginObject(std::string_view) override { ++objects; }
		void EndObject() override {}
		void BeginArray(std::string_view, std::size_t&) override { ++arrays; }
		void EndArray() override {}
	};

	// Fake reader: every array has 2 elements, every int32 is 7,
	// and strings are "key0", "key1"...
	class FakeReader final : public Archive
	{
	public:
		bool IsLoading() const override { return true; }

		void Primitive(std::string_view, bool&) override {}
		void Primitive(std::string_view, std::int32_t& value) override { value = 7; }
		void Primitive(std::string_view, std::uint32_t&) override {}
		void Primitive(std::string_view, std::int64_t&) override {}
		void Primitive(std::string_view, std::uint64_t&) override {}
		void Primitive(std::string_view, float&) override {}
		void Primitive(std::string_view, double&) override {}
		void Primitive(std::string_view, std::string& value) override { value = "key" + std::to_string(m_strings++); }

		void BeginObject(std::string_view) override {}
		void EndObject() override {}
		void BeginArray(std::string_view, std::size_t& size) override { size = 2; }
		void EndArray() override {}

	private:
		int m_strings = 0;
	};
}

TEST_CASE("Archive walks structs and nested arrays")
{
	CountingArchive archive;
	Hero hero;
	archive.Value("hero", hero);

	REQUIRE(archive.IsSaving());
	REQUIRE(archive.objects == 1);
	REQUIRE(archive.arrays == 3);
	REQUIRE(archive.primitives == 5);
	REQUIRE_FALSE(archive.HasFailed());
}

TEST_CASE("Archive fills containers when loading")
{
	FakeReader reader;

	std::vector<std::int32_t> values;
	reader.Value("values", values);
	REQUIRE(values == std::vector<std::int32_t>{7, 7});

	std::map<std::string, std::int32_t> scores;
	reader.Value("scores", scores);
	REQUIRE(scores.size() == 2);
	REQUIRE(scores.at("key0") == 7);
	REQUIRE(scores.at("key1") == 7);
}

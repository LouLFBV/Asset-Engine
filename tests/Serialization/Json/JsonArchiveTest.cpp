#include <AssetEngine/Core/IO/MemoryStream.h>
#include <AssetEngine/Serialization/Json/JsonParser.h>
#include <AssetEngine/Serialization/Json/JsonReader.h>
#include <AssetEngine/Serialization/Json/JsonWriter.h>
#include <AssetEngine/Serialization/Serialization.h>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <limits>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <vector>

using namespace AssetEngine;

namespace
{
	static_assert(!JsonWriter::Loading);
	static_assert(JsonReader::Loading);

	struct Stats
	{
		std::int32_t strength = 0;
		float speed = 0.0f;

		void Serialize(Archive& archive)
		{
			archive.Value("strength", strength);
			archive.Value("speed", speed);
		}

		bool operator==(const Stats&) const = default;
	};

	struct Hero
	{
		std::string name;
		bool alive = false;
		std::int64_t gold = 0;
		std::uint64_t maxGold = 0;
		double ratio = 0.0;
		Stats stats;
		std::vector<std::int32_t> emptyList;
		std::vector<std::vector<std::int32_t>> grid;
		std::map<std::string, std::int32_t> scores;
		std::map<std::int32_t, Stats> statsByLevel;

		void Serialize(Archive& archive)
		{
			archive.Value("name", name);
			archive.Value("alive", alive);
			archive.Value("gold", gold);
			archive.Value("maxGold", maxGold);
			archive.Value("ratio", ratio);
			archive.Value("stats", stats);
			archive.Value("emptyList", emptyList);
			archive.Value("grid", grid);
			archive.Value("scores", scores);
			archive.Value("statsByLevel", statsByLevel);
		}

		bool operator==(const Hero&) const = default;
	};

	Hero MakeHero()
	{
		Hero hero;
		hero.name = "Lyra \"the brave\"";
		hero.alive = true;
		hero.gold = -5000000000;
		hero.maxGold = std::numeric_limits<std::uint64_t>::max();
		hero.ratio = 0.1;
		hero.stats = {12, 0.1f};
		hero.grid = {{1, 2}, {}, {3}};
		hero.scores = {{"arena", 10}, {"quest", 20}};
		hero.statsByLevel = {{1, {5, 1.0f}}, {10, {50, 2.5f}}};
		return hero;
	}

	std::string ToJson(Hero hero)
	{
		MemoryStream stream;
		JsonWriter writer(stream);
		writer.Value("hero", hero);
		REQUIRE(writer.Finish());

		const auto bytes = stream.Data();
		return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
	}

	// Loads a Hero from JSON text, and returns the reader's error so it can be checked.
	ArchiveError LoadHero(const std::string& text, Hero& hero)
	{
		MemoryStream stream;
		REQUIRE(stream.Write(std::as_bytes(std::span(text.data(), text.size()))));
		REQUIRE(stream.Seek(0));

		JsonReader reader(stream);
		reader.Value("hero", hero);
		return reader.GetError();
	}
}

TEST_CASE("JSON archive round-trips a struct with every container")
{
	const Hero hero = MakeHero();

	Hero loaded;
	REQUIRE(LoadHero(ToJson(hero), loaded) == ArchiveError::None);
	REQUIRE(loaded == hero);
}

TEST_CASE("JSON archive writes readable text")
{
	const std::string text = ToJson(MakeHero());

	const std::optional<JsonValue> document = ParseJson(text);
	REQUIRE(document.has_value());

	const JsonValue* hero = document->Find("hero");
	REQUIRE(hero != nullptr);
	REQUIRE(hero->Find("name")->AsString() == "Lyra \"the brave\"");
	REQUIRE(hero->Find("stats")->Find("strength")->AsInt() == 12);

	// A map is written as an array of { "key", "value" } objects.
	const JsonValue& firstScore = hero->Find("scores")->AsArray()[0];
	REQUIRE(firstScore.Find("key")->AsString() == "arena");
	REQUIRE(firstScore.Find("value")->AsInt() == 10);
}

TEST_CASE("JSON archive reads fields by name, in any order")
{
	const std::string text = R"({ "hero": { "stats": { "speed": 2, "strength": 7 },
		"scores": [], "statsByLevel": [], "grid": [], "emptyList": [],
		"ratio": 0.5, "maxGold": 1, "gold": 2, "alive": true, "name": "Edited by hand" } })";

	Hero hero;
	REQUIRE(LoadHero(text, hero) == ArchiveError::None);
	REQUIRE(hero.name == "Edited by hand");
	REQUIRE(hero.stats.strength == 7);
	REQUIRE(hero.stats.speed == 2.0f);
}

TEST_CASE("JSON reader reports a missing field")
{
	Hero hero;
	REQUIRE(LoadHero(R"({ "hero": { "name": "Lyra" } })", hero) == ArchiveError::MissingField);
	REQUIRE(hero.name == "Lyra");
}

TEST_CASE("JSON reader reports a value of the wrong type")
{
	Hero hero;
	REQUIRE(LoadHero(R"({ "hero": { "name": 42 } })", hero) == ArchiveError::TypeMismatch);
}

TEST_CASE("JSON reader refuses a number too large for its field")
{
	const std::string text = R"({ "hero": { "name": "", "alive": false, "gold": 0, "maxGold": 0, "ratio": 0,
		"stats": { "strength": 5000000000, "speed": 0 } } })";

	Hero hero;
	REQUIRE(LoadHero(text, hero) == ArchiveError::InvalidData);
	REQUIRE(hero.stats.strength == 0);
}

TEST_CASE("JSON reader refuses text that is not JSON")
{
	Hero hero;
	REQUIRE(LoadHero("not json", hero) == ArchiveError::InvalidData);
	REQUIRE(LoadHero("[1, 2]", hero) == ArchiveError::InvalidData);
}

TEST_CASE("JSON writer refuses a number JSON cannot store")
{
	MemoryStream stream;
	JsonWriter writer(stream);

	double infinite = std::numeric_limits<double>::infinity();
	writer.Value("value", infinite);

	REQUIRE(writer.GetError() == ArchiveError::InvalidData);
	REQUIRE_FALSE(writer.Finish());
	REQUIRE(stream.Size() == 0);
}

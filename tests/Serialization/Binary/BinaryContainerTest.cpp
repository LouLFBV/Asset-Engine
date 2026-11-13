#include "BinaryTestUtils.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

using namespace AssetEngine;
using namespace AssetEngine::Tests;

namespace
{
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
			archive.Value("ratio", ratio);
			archive.Value("stats", stats);
			archive.Value("emptyList", emptyList);
			archive.Value("grid", grid);
			archive.Value("scores", scores);
			archive.Value("statsByLevel", statsByLevel);
		}

		bool operator==(const Hero&) const = default;
	};
}

TEST_CASE("Binary archive round-trips a struct with every container")
{
	Hero hero;
	hero.name = "Lyra";
	hero.alive = true;
	hero.gold = -5000000000;
	hero.ratio = 0.25;
	hero.stats = {12, 1.5f};
	hero.grid = {{1, 2}, {}, {3}};
	hero.scores = {{"arena", 10}, {"quest", 20}};
	hero.statsByLevel = {{1, {5, 1.0f}}, {10, {50, 2.0f}}};

	REQUIRE(BinaryRoundTrip(hero) == hero);
}

TEST_CASE("Binary archive round-trips nested arrays of structs")
{
	const std::vector<std::vector<Stats>> waves{{{1, 1.0f}, {2, 2.0f}}, {}, {{3, 3.0f}}};
	REQUIRE(BinaryRoundTrip(waves) == waves);
}

TEST_CASE("Binary archive writes only the values")
{
	// Field names and object boundaries are not written: 4 bytes for strength, 4 for speed.
	REQUIRE(BinaryBytes(Stats{7, 0.0f}) == Bytes({0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));

	// An empty array is just its size: one varint byte, 0.
	REQUIRE(BinaryBytes(std::vector<std::int32_t>{}) == Bytes({0x00}));

	// A map is its size, then each key and value.
	const std::map<std::string, std::int32_t> scores{{"A", 1}};
	REQUIRE(BinaryBytes(scores) == Bytes({0x01, 0x01, 0x41, 0x01, 0x00, 0x00, 0x00}));
}

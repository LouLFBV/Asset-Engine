#include <AssetEngine/Serialization/Json/JsonPrinter.h>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <limits>
#include <string>

using namespace AssetEngine;

TEST_CASE("JSON printer writes every simple value")
{
	REQUIRE(PrintJson(JsonValue()) == "null");
	REQUIRE(PrintJson(JsonValue(true)) == "true");
	REQUIRE(PrintJson(JsonValue(false)) == "false");
	REQUIRE(PrintJson(JsonValue(std::int64_t{-42})) == "-42");
	REQUIRE(PrintJson(JsonValue(std::numeric_limits<std::uint64_t>::max())) == "18446744073709551615");
	REQUIRE(PrintJson(JsonValue(0.5)) == "0.5");
	REQUIRE(PrintJson(JsonValue(std::string("Lyra"))) == "\"Lyra\"");
}

TEST_CASE("JSON printer keeps doubles recognisable")
{
	// A whole double keeps its ".0", so it is read back as a double.
	REQUIRE(PrintJson(JsonValue(2.0)) == "2.0");
	REQUIRE(PrintJson(JsonValue(1e300)) == "1e+300");
}

TEST_CASE("JSON printer escapes special characters")
{
	REQUIRE(PrintJson(JsonValue(std::string("a\"b\\c"))) == "\"a\\\"b\\\\c\"");
	REQUIRE(PrintJson(JsonValue(std::string("line\nnext\ttab"))) == "\"line\\nnext\\ttab\"");
	REQUIRE(PrintJson(JsonValue(std::string("\x01"))) == "\"\\u0001\"");

	// UTF-8 accents are valid JSON text: they are copied without escaping.
	REQUIRE(PrintJson(JsonValue(std::string("\xC3\xA9"))) == "\"\xC3\xA9\"");
}

TEST_CASE("JSON printer indents arrays and objects")
{
	JsonValue hero = JsonValue::MakeObject();
	hero.Add("name", JsonValue(std::string("Lyra")));

	JsonValue& scores = hero.Add("scores", JsonValue::MakeArray());
	scores.Append(JsonValue(std::int64_t{1}));
	scores.Append(JsonValue(std::int64_t{2}));

	hero.Add("tags", JsonValue::MakeArray());
	hero.Add("stats", JsonValue::MakeObject());

	const std::string expected =
		"{\n"
		"  \"name\": \"Lyra\",\n"
		"  \"scores\": [\n"
		"    1,\n"
		"    2\n"
		"  ],\n"
		"  \"tags\": [],\n"
		"  \"stats\": {}\n"
		"}";

	REQUIRE(PrintJson(hero) == expected);
}

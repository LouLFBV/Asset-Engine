#include <AssetEngine/Serialization/Json/JsonParser.h>
#include <AssetEngine/Serialization/Json/JsonPrinter.h>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <limits>
#include <optional>
#include <string>

using namespace AssetEngine;

namespace
{
	bool IsInvalid(const std::string& text)
	{
		return !ParseJson(text).has_value();
	}
}

TEST_CASE("JSON parser reads every simple value")
{
	REQUIRE(ParseJson("null")->Is(JsonValue::Type::Null));
	REQUIRE(ParseJson("true")->AsBool());
	REQUIRE_FALSE(ParseJson("false")->AsBool());
	REQUIRE(ParseJson("-42")->AsInt() == -42);
	REQUIRE(ParseJson("18446744073709551615")->AsUInt() == std::numeric_limits<std::uint64_t>::max());
	REQUIRE(ParseJson("0.5")->AsDouble() == 0.5);
	REQUIRE(ParseJson("-1.5e3")->AsDouble() == -1500.0);
	REQUIRE(ParseJson("\"Lyra\"")->AsString() == "Lyra");
}

TEST_CASE("JSON parser reads arrays and objects with spaces")
{
	const std::optional<JsonValue> value = ParseJson(" { \"name\" : \"Lyra\" , \"scores\" : [ 1 , 2 ] , \"empty\" : {} } ");
	REQUIRE(value.has_value());
	REQUIRE(value->Find("name")->AsString() == "Lyra");
	REQUIRE(value->Find("scores")->AsArray().size() == 2);
	REQUIRE(value->Find("scores")->AsArray()[1].AsInt() == 2);
	REQUIRE(value->Find("empty")->AsObject().empty());
	REQUIRE(value->Find("missing") == nullptr);
}

TEST_CASE("JSON parser decodes escapes")
{
	REQUIRE(ParseJson(R"("a\"b\\c\/d\n")")->AsString() == "a\"b\\c/d\n");

	// é is "é", written in UTF-8 as two bytes.
	REQUIRE(ParseJson(R"("é")")->AsString() == "\xC3\xA9");

	// An emoji is beyond U+FFFF: two escapes (a surrogate pair) become four UTF-8 bytes.
	REQUIRE(ParseJson(R"("😀")")->AsString() == "\xF0\x9F\x98\x80");
}

TEST_CASE("JSON parser reads back what the printer writes")
{
	JsonValue hero = JsonValue::MakeObject();
	hero.Add("name", JsonValue(std::string("L\"y\\ra\n")));
	hero.Add("gold", JsonValue(std::int64_t{-5000000000}));
	hero.Add("max", JsonValue(std::numeric_limits<std::uint64_t>::max()));
	hero.Add("ratio", JsonValue(0.1));
	hero.Add("whole", JsonValue(2.0));
	hero.Add("nothing", JsonValue());

	JsonValue& grid = hero.Add("grid", JsonValue::MakeArray());
	grid.Append(JsonValue::MakeArray()).Append(JsonValue(true));
	grid.Append(JsonValue::MakeArray());

	REQUIRE(ParseJson(PrintJson(hero)) == hero);
}

TEST_CASE("JSON parser refuses invalid text")
{
	REQUIRE(IsInvalid(""));
	REQUIRE(IsInvalid("{"));
	REQUIRE(IsInvalid("[1,]"));
	REQUIRE(IsInvalid("{\"a\" 1}"));
	REQUIRE(IsInvalid("{a: 1}"));
	REQUIRE(IsInvalid("01"));
	REQUIRE(IsInvalid("1."));
	REQUIRE(IsInvalid("tru"));
	REQUIRE(IsInvalid("1 2"));
	REQUIRE(IsInvalid("\"unterminated"));
	REQUIRE(IsInvalid("\"bad \\x escape\""));
	REQUIRE(IsInvalid("\"raw\x01control\""));
	REQUIRE(IsInvalid(R"("\uDE00")"));
}

TEST_CASE("JSON parser reports where the error is")
{
	JsonParseError error;
	REQUIRE_FALSE(ParseJson("[1, 2,, 3]", &error).has_value());
	REQUIRE(error.position == 6);
	REQUIRE_FALSE(error.message.empty());
}

TEST_CASE("JSON parser refuses deep nesting without crashing")
{
	const std::string deep = std::string(100000, '[') + std::string(100000, ']');
	REQUIRE(IsInvalid(deep));

	const std::string reasonable = std::string(100, '[') + std::string(100, ']');
	REQUIRE_FALSE(IsInvalid(reasonable));
}

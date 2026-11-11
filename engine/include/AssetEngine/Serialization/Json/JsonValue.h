#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace AssetEngine
{
	struct JsonMember;

	// One JSON value: null, a boolean, a number, a string, an array or an object.
	class JsonValue
	{
	public:
		// Same order as the alternatives of m_data, so GetType() is the variant index.
		enum class Type
		{
			Null,
			Bool,
			Int,
			UInt,
			Double,
			String,
			Array,
			Object
		};

		using Array = std::vector<JsonValue>;
		using Object = std::vector<JsonMember>;

		JsonValue() = default;
		explicit JsonValue(bool value);
		explicit JsonValue(std::int64_t value);
		explicit JsonValue(std::uint64_t value);
		explicit JsonValue(double value);
		explicit JsonValue(std::string value);

		static JsonValue MakeArray();
		static JsonValue MakeObject();

		Type GetType() const;
		bool Is(Type type) const { return GetType() == type; }

		// Each accessor must only be called after checking the type with Is().
		bool AsBool() const;
		std::int64_t AsInt() const;
		std::uint64_t AsUInt() const;
		double AsDouble() const;
		const std::string& AsString() const;
		const Array& AsArray() const;
		const Object& AsObject() const;

		JsonValue& Append(JsonValue value);
		JsonValue& Add(std::string key, JsonValue value);
		const JsonValue* Find(std::string_view key) const;

		friend bool operator==(const JsonValue& left, const JsonValue& right);

	private:
		std::variant<std::monostate, bool, std::int64_t, std::uint64_t, double, std::string, Array, Object> m_data;
	};

	// One "key": value pair of an object. Objects keep their members in insertion order.
	struct JsonMember
	{
		std::string key;
		JsonValue value;

		bool operator==(const JsonMember&) const = default;
	};
}

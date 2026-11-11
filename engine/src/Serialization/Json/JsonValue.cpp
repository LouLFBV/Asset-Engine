#include <AssetEngine/Serialization/Json/JsonValue.h>

#include <utility>

namespace AssetEngine
{
	JsonValue::JsonValue(bool value)
		: m_data(value)
	{
	}

	JsonValue::JsonValue(std::int64_t value)
		: m_data(value)
	{
	}

	JsonValue::JsonValue(std::uint64_t value)
		: m_data(value)
	{
	}

	JsonValue::JsonValue(double value)
		: m_data(value)
	{
	}

	JsonValue::JsonValue(std::string value)
		: m_data(std::move(value))
	{
	}

	JsonValue JsonValue::MakeArray()
	{
		JsonValue value;
		value.m_data = Array{};
		return value;
	}

	JsonValue JsonValue::MakeObject()
	{
		JsonValue value;
		value.m_data = Object{};
		return value;
	}

	JsonValue::Type JsonValue::GetType() const
	{
		return static_cast<Type>(m_data.index());
	}

	bool JsonValue::AsBool() const
	{
		return std::get<bool>(m_data);
	}

	std::int64_t JsonValue::AsInt() const
	{
		return std::get<std::int64_t>(m_data);
	}

	std::uint64_t JsonValue::AsUInt() const
	{
		return std::get<std::uint64_t>(m_data);
	}

	double JsonValue::AsDouble() const
	{
		return std::get<double>(m_data);
	}

	const std::string& JsonValue::AsString() const
	{
		return std::get<std::string>(m_data);
	}

	const JsonValue::Array& JsonValue::AsArray() const
	{
		return std::get<Array>(m_data);
	}

	const JsonValue::Object& JsonValue::AsObject() const
	{
		return std::get<Object>(m_data);
	}

	JsonValue& JsonValue::Append(JsonValue value)
	{
		return std::get<Array>(m_data).emplace_back(std::move(value));
	}

	JsonValue& JsonValue::Add(std::string key, JsonValue value)
	{
		return std::get<Object>(m_data).emplace_back(JsonMember{std::move(key), std::move(value)}).value;
	}

	const JsonValue* JsonValue::Find(std::string_view key) const
	{
		for (const JsonMember& member : std::get<Object>(m_data))
		{
			if (member.key == key)
				return &member.value;
		}

		return nullptr;
	}

	bool operator==(const JsonValue& left, const JsonValue& right)
	{
		return left.m_data == right.m_data;
	}
}

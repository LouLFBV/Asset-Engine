#include <AssetEngine/Serialization/Json/JsonWriter.h>

#include <AssetEngine/Serialization/Json/JsonPrinter.h>

#include <cmath>
#include <span>
#include <utility>

namespace AssetEngine
{
	JsonWriter::JsonWriter(IOutputStream& stream)
		: m_stream(stream)
	{
		m_stack.push_back(&m_root);
	}

	void JsonWriter::Primitive(std::string_view name, bool& value)
	{
		Insert(name, JsonValue(value));
	}

	void JsonWriter::Primitive(std::string_view name, std::int32_t& value)
	{
		Insert(name, JsonValue(static_cast<std::int64_t>(value)));
	}

	void JsonWriter::Primitive(std::string_view name, std::uint32_t& value)
	{
		Insert(name, JsonValue(static_cast<std::uint64_t>(value)));
	}

	void JsonWriter::Primitive(std::string_view name, std::int64_t& value)
	{
		Insert(name, JsonValue(value));
	}

	void JsonWriter::Primitive(std::string_view name, std::uint64_t& value)
	{
		Insert(name, JsonValue(value));
	}

	void JsonWriter::Primitive(std::string_view name, float& value)
	{
		InsertNumber(name, static_cast<double>(value));
	}

	void JsonWriter::Primitive(std::string_view name, double& value)
	{
		InsertNumber(name, value);
	}

	void JsonWriter::Primitive(std::string_view name, std::string& value)
	{
		Insert(name, JsonValue(value));
	}

	void JsonWriter::BeginObject(std::string_view name)
	{
		m_stack.push_back(&Insert(name, JsonValue::MakeObject()));
	}

	void JsonWriter::EndObject()
	{
		if (m_stack.size() > 1)
			m_stack.pop_back();
	}

	void JsonWriter::BeginArray(std::string_view name, std::size_t&)
	{
		m_stack.push_back(&Insert(name, JsonValue::MakeArray()));
	}

	void JsonWriter::EndArray()
	{
		if (m_stack.size() > 1)
			m_stack.pop_back();
	}

	bool JsonWriter::Finish()
	{
		if (HasFailed())
			return false;

		const std::string text = PrintJson(m_root);
		if (!m_stream.Write(std::as_bytes(std::span(text.data(), text.size()))))
		{
			Fail(ArchiveError::StreamError);
			return false;
		}

		return true;
	}

	JsonValue& JsonWriter::Insert(std::string_view name, JsonValue value)
	{
		JsonValue& parent = *m_stack.back();

		// Inside an object, a value is a named member. Inside an array, it is the next item.
		if (parent.Is(JsonValue::Type::Object))
			return parent.Add(std::string(name), std::move(value));

		return parent.Append(std::move(value));
	}

	void JsonWriter::InsertNumber(std::string_view name, double value)
	{
		// JSON has no infinity or NaN: such a value cannot be saved.
		if (!std::isfinite(value))
		{
			Fail(ArchiveError::InvalidData);
			Insert(name, JsonValue());
			return;
		}

		Insert(name, JsonValue(value));
	}
}

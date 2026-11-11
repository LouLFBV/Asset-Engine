#include <AssetEngine/Serialization/Json/JsonReader.h>

#include <AssetEngine/Serialization/Json/JsonParser.h>

#include <optional>
#include <span>

namespace AssetEngine
{
	JsonReader::JsonReader(IInputStream& stream)
	{
		// Read the whole stream, whatever its position, in fixed-size chunks.
		std::string text;
		std::byte chunk[4096];
		while (const std::size_t count = stream.Read(chunk))
			text.append(reinterpret_cast<const char*>(chunk), count);

		std::optional<JsonValue> document = ParseJson(text);
		if (!document || !document->Is(JsonValue::Type::Object))
		{
			Fail(ArchiveError::InvalidData);
			m_stack.push_back(Frame{});
			return;
		}

		m_root = std::move(*document);
		m_stack.push_back(Frame{&m_root});
	}

	void JsonReader::Primitive(std::string_view name, bool& value)
	{
		if (const JsonValue* node = NextOfType(name, JsonValue::Type::Bool))
			value = node->AsBool();
	}

	void JsonReader::Primitive(std::string_view name, std::int32_t& value)
	{
		ReadInteger(name, value);
	}

	void JsonReader::Primitive(std::string_view name, std::uint32_t& value)
	{
		ReadInteger(name, value);
	}

	void JsonReader::Primitive(std::string_view name, std::int64_t& value)
	{
		ReadInteger(name, value);
	}

	void JsonReader::Primitive(std::string_view name, std::uint64_t& value)
	{
		ReadInteger(name, value);
	}

	void JsonReader::Primitive(std::string_view name, float& value)
	{
		ReadFloating(name, value);
	}

	void JsonReader::Primitive(std::string_view name, double& value)
	{
		ReadFloating(name, value);
	}

	void JsonReader::Primitive(std::string_view name, std::string& value)
	{
		if (const JsonValue* node = NextOfType(name, JsonValue::Type::String))
			value = node->AsString();
	}

	void JsonReader::BeginObject(std::string_view name)
	{
		// Always push a frame, even on error, so every End call still matches its Begin.
		m_stack.push_back(Frame{NextOfType(name, JsonValue::Type::Object)});
	}

	void JsonReader::EndObject()
	{
		if (m_stack.size() > 1)
			m_stack.pop_back();
	}

	void JsonReader::BeginArray(std::string_view name, std::size_t& size)
	{
		const JsonValue* node = NextOfType(name, JsonValue::Type::Array);
		size = node != nullptr ? node->AsArray().size() : 0;
		m_stack.push_back(Frame{node});
	}

	void JsonReader::EndArray()
	{
		if (m_stack.size() > 1)
			m_stack.pop_back();
	}

	const JsonValue* JsonReader::Next(std::string_view name)
	{
		if (HasFailed())
			return nullptr;

		Frame& frame = m_stack.back();
		if (frame.value == nullptr)
		{
			Fail(ArchiveError::InvalidData);
			return nullptr;
		}

		// Inside an object, a value is found by its name.
		if (frame.value->Is(JsonValue::Type::Object))
		{
			const JsonValue* member = frame.value->Find(name);
			if (member == nullptr)
				Fail(ArchiveError::MissingField);
			return member;
		}

		// Inside an array, values are read in order and the name is ignored.
		const JsonValue::Array& items = frame.value->AsArray();
		if (frame.nextItem >= items.size())
		{
			Fail(ArchiveError::EndOfData);
			return nullptr;
		}

		return &items[frame.nextItem++];
	}

	const JsonValue* JsonReader::NextOfType(std::string_view name, JsonValue::Type type)
	{
		const JsonValue* node = Next(name);
		if (node != nullptr && !node->Is(type))
		{
			Fail(ArchiveError::TypeMismatch);
			return nullptr;
		}

		return node;
	}
}

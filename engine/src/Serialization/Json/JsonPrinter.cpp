#include <AssetEngine/Serialization/Json/JsonPrinter.h>

#include <charconv>
#include <cmath>
#include <cstddef>
#include <string_view>

namespace AssetEngine
{
	namespace
	{
		void AppendIndent(std::string& out, std::size_t depth)
		{
			out.append(depth * 2, ' ');
		}

		void AppendString(std::string& out, std::string_view text)
		{
			constexpr char hexDigits[] = "0123456789ABCDEF";

			out += '"';
			for (const char c : text)
			{
				switch (c)
				{
				case '"': out += "\\\""; break;
				case '\\': out += "\\\\"; break;
				case '\b': out += "\\b"; break;
				case '\f': out += "\\f"; break;
				case '\n': out += "\\n"; break;
				case '\r': out += "\\r"; break;
				case '\t': out += "\\t"; break;
				default:
					if (static_cast<unsigned char>(c) < 0x20)
					{
						// Other control characters are not allowed raw in JSON: write them as \u00XX.
						out += "\\u00";
						out += hexDigits[(c >> 4) & 0x0F];
						out += hexDigits[c & 0x0F];
					}
					else
					{
						// Every other byte, including UTF-8 accents, is copied as is.
						out += c;
					}
				}
			}
			out += '"';
		}

		template <typename T>
		void AppendNumber(std::string& out, T number)
		{
			char buffer[32];
			const auto result = std::to_chars(buffer, buffer + sizeof(buffer), number);
			out.append(buffer, result.ptr);
		}

		void AppendDouble(std::string& out, double number)
		{
			// JSON has no infinity or NaN.
			if (!std::isfinite(number))
			{
				out += "null";
				return;
			}

			// to_chars writes the shortest text that reads back to the exact same double.
			char buffer[32];
			const auto result = std::to_chars(buffer, buffer + sizeof(buffer), number);
			const std::string_view text(buffer, static_cast<std::size_t>(result.ptr - buffer));
			out += text;

			// Keep "2.0" instead of "2", so the value is read back as a double, not an integer.
			if (text.find_first_of(".e") == std::string_view::npos)
				out += ".0";
		}

		void AppendValue(std::string& out, const JsonValue& value, std::size_t depth)
		{
			switch (value.GetType())
			{
			case JsonValue::Type::Null: out += "null"; break;
			case JsonValue::Type::Bool: out += value.AsBool() ? "true" : "false"; break;
			case JsonValue::Type::Int: AppendNumber(out, value.AsInt()); break;
			case JsonValue::Type::UInt: AppendNumber(out, value.AsUInt()); break;
			case JsonValue::Type::Double: AppendDouble(out, value.AsDouble()); break;
			case JsonValue::Type::String: AppendString(out, value.AsString()); break;

			case JsonValue::Type::Array:
			{
				const auto& items = value.AsArray();
				if (items.empty())
				{
					out += "[]";
					break;
				}

				out += "[\n";
				for (std::size_t i = 0; i < items.size(); ++i)
				{
					AppendIndent(out, depth + 1);
					AppendValue(out, items[i], depth + 1);
					out += i + 1 < items.size() ? ",\n" : "\n";
				}
				AppendIndent(out, depth);
				out += ']';
				break;
			}

			case JsonValue::Type::Object:
			{
				const auto& members = value.AsObject();
				if (members.empty())
				{
					out += "{}";
					break;
				}

				out += "{\n";
				for (std::size_t i = 0; i < members.size(); ++i)
				{
					AppendIndent(out, depth + 1);
					AppendString(out, members[i].key);
					out += ": ";
					AppendValue(out, members[i].value, depth + 1);
					out += i + 1 < members.size() ? ",\n" : "\n";
				}
				AppendIndent(out, depth);
				out += '}';
				break;
			}
			}
		}
	}

	std::string PrintJson(const JsonValue& value)
	{
		std::string out;
		AppendValue(out, value, 0);
		return out;
	}
}

#include <AssetEngine/Serialization/Json/JsonParser.h>

#include <charconv>
#include <cstdint>
#include <limits>
#include <system_error>
#include <utility>

namespace AssetEngine
{
	namespace
	{
		// Deeper nesting is refused, so a malicious file cannot overflow the call stack.
		constexpr std::size_t MaxDepth = 256;

		class Parser
		{
		public:
			explicit Parser(std::string_view text)
				: m_text(text)
			{
			}

			std::optional<JsonValue> ParseDocument()
			{
				SkipWhitespace();
				std::optional<JsonValue> value = ParseValue(0);
				if (!value)
					return std::nullopt;

				SkipWhitespace();
				if (m_position != m_text.size())
					return Fail("unexpected text after the value");

				return value;
			}

			const JsonParseError& Error() const { return m_error; }

		private:
			std::nullopt_t Fail(const char* message)
			{
				m_error.position = m_position;
				m_error.message = message;
				return std::nullopt;
			}

			bool AtEnd() const { return m_position >= m_text.size(); }
			char Peek() const { return m_text[m_position]; }

			void SkipWhitespace()
			{
				while (!AtEnd() && (Peek() == ' ' || Peek() == '\t' || Peek() == '\n' || Peek() == '\r'))
					++m_position;
			}

			bool Consume(std::string_view word)
			{
				if (m_text.substr(m_position, word.size()) != word)
					return false;

				m_position += word.size();
				return true;
			}

			std::optional<JsonValue> ParseValue(std::size_t depth)
			{
				if (depth > MaxDepth)
					return Fail("too deeply nested");

				if (AtEnd())
					return Fail("unexpected end of text");

				switch (Peek())
				{
				case '{': return ParseObject(depth);
				case '[': return ParseArray(depth);
				case '"':
				{
					std::optional<std::string> text = ParseString();
					if (!text)
						return std::nullopt;
					return JsonValue(std::move(*text));
				}
				case 't': return Consume("true") ? std::optional(JsonValue(true)) : Fail("invalid literal");
				case 'f': return Consume("false") ? std::optional(JsonValue(false)) : Fail("invalid literal");
				case 'n': return Consume("null") ? std::optional(JsonValue()) : Fail("invalid literal");
				default: return ParseNumber();
				}
			}

			std::optional<JsonValue> ParseObject(std::size_t depth)
			{
				++m_position;
				JsonValue object = JsonValue::MakeObject();

				SkipWhitespace();
				if (!AtEnd() && Peek() == '}')
				{
					++m_position;
					return object;
				}

				while (true)
				{
					SkipWhitespace();
					if (AtEnd() || Peek() != '"')
						return Fail("expected a key");

					std::optional<std::string> key = ParseString();
					if (!key)
						return std::nullopt;

					SkipWhitespace();
					if (AtEnd() || Peek() != ':')
						return Fail("expected ':'");
					++m_position;

					SkipWhitespace();
					std::optional<JsonValue> value = ParseValue(depth + 1);
					if (!value)
						return std::nullopt;

					object.Add(std::move(*key), std::move(*value));

					SkipWhitespace();
					if (AtEnd())
						return Fail("unexpected end of text");

					if (Peek() == '}')
					{
						++m_position;
						return object;
					}

					if (Peek() != ',')
						return Fail("expected ',' or '}'");
					++m_position;
				}
			}

			std::optional<JsonValue> ParseArray(std::size_t depth)
			{
				++m_position;
				JsonValue array = JsonValue::MakeArray();

				SkipWhitespace();
				if (!AtEnd() && Peek() == ']')
				{
					++m_position;
					return array;
				}

				while (true)
				{
					SkipWhitespace();
					std::optional<JsonValue> value = ParseValue(depth + 1);
					if (!value)
						return std::nullopt;

					array.Append(std::move(*value));

					SkipWhitespace();
					if (AtEnd())
						return Fail("unexpected end of text");

					if (Peek() == ']')
					{
						++m_position;
						return array;
					}

					if (Peek() != ',')
						return Fail("expected ',' or ']'");
					++m_position;
				}
			}

			std::optional<JsonValue> ParseNumber()
			{
				const std::size_t start = m_position;
				bool isInteger = true;

				if (!AtEnd() && Peek() == '-')
					++m_position;

				// JSON forbids leading zeros: "0" is valid, "01" is not.
				if (!AtEnd() && Peek() == '0')
					++m_position;
				else if (!SkipDigits())
					return Fail("invalid number");

				if (!AtEnd() && Peek() == '.')
				{
					isInteger = false;
					++m_position;
					if (!SkipDigits())
						return Fail("expected digits after '.'");
				}

				if (!AtEnd() && (Peek() == 'e' || Peek() == 'E'))
				{
					isInteger = false;
					++m_position;
					if (!AtEnd() && (Peek() == '+' || Peek() == '-'))
						++m_position;
					if (!SkipDigits())
						return Fail("expected digits in the exponent");
				}

				const char* first = m_text.data() + start;
				const char* last = m_text.data() + m_position;

				if (isInteger)
				{
					if (*first == '-')
					{
						std::int64_t value = 0;
						if (std::from_chars(first, last, value).ec == std::errc{})
							return JsonValue(value);
					}
					else
					{
						std::uint64_t value = 0;
						if (std::from_chars(first, last, value).ec == std::errc{})
						{
							// Prefer the signed form when the number fits, so small numbers are always Int.
							if (value <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))
								return JsonValue(static_cast<std::int64_t>(value));
							return JsonValue(value);
						}
					}
					// Too large for 64 bits: keep it as a double instead of failing.
				}

				double value = 0.0;
				if (std::from_chars(first, last, value).ec != std::errc{})
				{
					m_position = start;
					return Fail("number out of range");
				}

				return JsonValue(value);
			}

			bool SkipDigits()
			{
				const std::size_t start = m_position;
				while (!AtEnd() && Peek() >= '0' && Peek() <= '9')
					++m_position;

				return m_position > start;
			}

			std::optional<std::string> ParseString()
			{
				++m_position;
				std::string result;

				while (true)
				{
					if (AtEnd())
						return Fail("unterminated string");

					const char c = m_text[m_position++];
					if (c == '"')
						return result;

					if (static_cast<unsigned char>(c) < 0x20)
						return Fail("control character in a string");

					if (c != '\\')
					{
						result += c;
						continue;
					}

					if (AtEnd())
						return Fail("unterminated string");

					switch (m_text[m_position++])
					{
					case '"': result += '"'; break;
					case '\\': result += '\\'; break;
					case '/': result += '/'; break;
					case 'b': result += '\b'; break;
					case 'f': result += '\f'; break;
					case 'n': result += '\n'; break;
					case 'r': result += '\r'; break;
					case 't': result += '\t'; break;
					case 'u':
						if (!ParseUnicodeEscape(result))
							return std::nullopt;
						break;
					default:
						return Fail("invalid escape sequence");
					}
				}
			}

			std::optional<std::uint32_t> ParseHex4()
			{
				if (m_position + 4 > m_text.size())
				{
					Fail("incomplete \\u escape");
					return std::nullopt;
				}

				std::uint32_t value = 0;
				const char* first = m_text.data() + m_position;
				const auto result = std::from_chars(first, first + 4, value, 16);
				if (result.ec != std::errc{} || result.ptr != first + 4)
				{
					Fail("invalid \\u escape");
					return std::nullopt;
				}

				m_position += 4;
				return value;
			}

			bool ParseUnicodeEscape(std::string& out)
			{
				std::optional<std::uint32_t> unit = ParseHex4();
				if (!unit)
					return false;

				std::uint32_t codePoint = *unit;

				// Characters beyond U+FFFF are written as two escapes: a high then a low surrogate.
				if (codePoint >= 0xD800 && codePoint <= 0xDBFF)
				{
					if (!Consume("\\u"))
					{
						Fail("missing low surrogate");
						return false;
					}

					std::optional<std::uint32_t> low = ParseHex4();
					if (!low)
						return false;

					if (*low < 0xDC00 || *low > 0xDFFF)
					{
						Fail("invalid low surrogate");
						return false;
					}

					codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (*low - 0xDC00);
				}
				else if (codePoint >= 0xDC00 && codePoint <= 0xDFFF)
				{
					Fail("unexpected low surrogate");
					return false;
				}

				AppendUtf8(out, codePoint);
				return true;
			}

			static void AppendUtf8(std::string& out, std::uint32_t codePoint)
			{
				if (codePoint < 0x80)
				{
					out += static_cast<char>(codePoint);
				}
				else if (codePoint < 0x800)
				{
					out += static_cast<char>(0xC0 | (codePoint >> 6));
					out += static_cast<char>(0x80 | (codePoint & 0x3F));
				}
				else if (codePoint < 0x10000)
				{
					out += static_cast<char>(0xE0 | (codePoint >> 12));
					out += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
					out += static_cast<char>(0x80 | (codePoint & 0x3F));
				}
				else
				{
					out += static_cast<char>(0xF0 | (codePoint >> 18));
					out += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
					out += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
					out += static_cast<char>(0x80 | (codePoint & 0x3F));
				}
			}

			std::string_view m_text;
			std::size_t m_position = 0;
			JsonParseError m_error;
		};
	}

	std::optional<JsonValue> ParseJson(std::string_view text, JsonParseError* error)
	{
		Parser parser(text);
		std::optional<JsonValue> value = parser.ParseDocument();

		if (!value && error != nullptr)
			*error = parser.Error();

		return value;
	}
}

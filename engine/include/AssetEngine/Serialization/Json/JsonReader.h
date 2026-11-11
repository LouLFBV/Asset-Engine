#pragma once

#include <AssetEngine/Core/IO/IInputStream.h>
#include <AssetEngine/Serialization/Archive.h>
#include <AssetEngine/Serialization/Json/JsonValue.h>

#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace AssetEngine
{
	// Parses the whole JSON text of the stream, then hands out its values by name.
	class JsonReader final : public Archive
	{
	public:
		static constexpr bool Loading = true;

		explicit JsonReader(IInputStream& stream);

		// m_stack points into m_root: a copy would point into the original.
		JsonReader(const JsonReader&) = delete;
		JsonReader& operator=(const JsonReader&) = delete;

		bool IsLoading() const override { return Loading; }

		void Primitive(std::string_view name, bool& value) override;
		void Primitive(std::string_view name, std::int32_t& value) override;
		void Primitive(std::string_view name, std::uint32_t& value) override;
		void Primitive(std::string_view name, std::int64_t& value) override;
		void Primitive(std::string_view name, std::uint64_t& value) override;
		void Primitive(std::string_view name, float& value) override;
		void Primitive(std::string_view name, double& value) override;
		void Primitive(std::string_view name, std::string& value) override;

		void BeginObject(std::string_view name) override;
		void EndObject() override;
		void BeginArray(std::string_view name, std::size_t& size) override;
		void EndArray() override;

	private:
		// The object or array being read, and the next item to read in an array.
		struct Frame
		{
			const JsonValue* value = nullptr;
			std::size_t nextItem = 0;
		};

		const JsonValue* Next(std::string_view name);
		const JsonValue* NextOfType(std::string_view name, JsonValue::Type type);

		template <std::integral T>
		void ReadInteger(std::string_view name, T& value)
		{
			const JsonValue* node = Next(name);
			if (node == nullptr)
				return;

			if (node->Is(JsonValue::Type::Int))
				AssignInRange(value, node->AsInt());
			else if (node->Is(JsonValue::Type::UInt))
				AssignInRange(value, node->AsUInt());
			else
				Fail(ArchiveError::TypeMismatch);
		}

		template <std::integral T, std::integral Source>
		void AssignInRange(T& value, Source number)
		{
			// A number too large for the field, like 5 000 000 000 for an int32, is refused.
			if (!std::in_range<T>(number))
			{
				Fail(ArchiveError::InvalidData);
				return;
			}

			value = static_cast<T>(number);
		}

		template <std::floating_point T>
		void ReadFloating(std::string_view name, T& value)
		{
			const JsonValue* node = Next(name);
			if (node == nullptr)
				return;

			// A whole number such as 3 is also a valid float.
			double number = 0.0;
			switch (node->GetType())
			{
			case JsonValue::Type::Double: number = node->AsDouble(); break;
			case JsonValue::Type::Int: number = static_cast<double>(node->AsInt()); break;
			case JsonValue::Type::UInt: number = static_cast<double>(node->AsUInt()); break;
			default: Fail(ArchiveError::TypeMismatch); return;
			}

			// Converting a double too large for a float is undefined behaviour: refuse it.
			if (std::abs(number) > static_cast<double>(std::numeric_limits<T>::max()))
			{
				Fail(ArchiveError::InvalidData);
				return;
			}

			value = static_cast<T>(number);
		}

		JsonValue m_root;
		std::vector<Frame> m_stack;
	};
}

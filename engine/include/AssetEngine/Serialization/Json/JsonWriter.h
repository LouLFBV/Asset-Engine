#pragma once

#include <AssetEngine/Core/IO/IOutputStream.h>
#include <AssetEngine/Serialization/Archive.h>
#include <AssetEngine/Serialization/Json/JsonValue.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace AssetEngine
{
	// Builds a JSON tree from the values it receives, then writes it as text in Finish().
	class JsonWriter final : public Archive
	{
	public:
		static constexpr bool Loading = false;

		explicit JsonWriter(IOutputStream& stream);

		// m_stack points into m_root: a copy would point into the original.
		JsonWriter(const JsonWriter&) = delete;
		JsonWriter& operator=(const JsonWriter&) = delete;

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

		// Writes the whole JSON text to the stream. Call it once, after serializing.
		bool Finish();

	private:
		JsonValue& Insert(std::string_view name, JsonValue value);
		void InsertNumber(std::string_view name, double value);

		IOutputStream& m_stream;
		JsonValue m_root = JsonValue::MakeObject();
		std::vector<JsonValue*> m_stack;
	};
}

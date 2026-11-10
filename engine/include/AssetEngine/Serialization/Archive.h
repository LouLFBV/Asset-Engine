#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace AssetEngine
{
	class Archive;

	enum class ArchiveError
	{
		None,
		EndOfData,
		InvalidData,
		StreamError,
		MissingField,
		TypeMismatch
	};

	template <typename T>
	struct Serializer;

	template <typename T>
	concept Serializable = requires(Archive& archive, std::string_view name, T& value)
	{
		Serializer<T>::Process(archive, name, value);
	};

	class Archive
	{
	public:
		virtual ~Archive() = default;

		virtual bool IsLoading() const = 0;
		bool IsSaving() const { return !IsLoading(); }

		template <Serializable T>
		void Value(std::string_view name, T& value)
		{
			Serializer<T>::Process(*this, name, value);
		}

		virtual void Primitive(std::string_view name, bool& value) = 0;
		virtual void Primitive(std::string_view name, std::int32_t& value) = 0;
		virtual void Primitive(std::string_view name, std::uint32_t& value) = 0;
		virtual void Primitive(std::string_view name, std::int64_t& value) = 0;
		virtual void Primitive(std::string_view name, std::uint64_t& value) = 0;
		virtual void Primitive(std::string_view name, float& value) = 0;
		virtual void Primitive(std::string_view name, double& value) = 0;
		virtual void Primitive(std::string_view name, std::string& value) = 0;

		virtual void BeginObject(std::string_view name) = 0;
		virtual void EndObject() = 0;
		virtual void BeginArray(std::string_view name, std::size_t& size) = 0;
		virtual void EndArray() = 0;

		ArchiveError GetError() const { return m_error; }
		bool HasFailed() const { return m_error != ArchiveError::None; }

	protected:
		void Fail(ArchiveError error)
		{
			if (m_error == ArchiveError::None)
				m_error = error;
		}

	private:
		ArchiveError m_error = ArchiveError::None;
	};
}

#pragma once

#include <AssetEngine/Core/Reflection/TypeId.h>

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace AssetEngine
{
	class TypeInfo;

	// One direct base of a type, and how to turn a pointer to the type into a pointer to that base.
	struct BaseInfo
	{
		const TypeInfo* type = nullptr;
		void* (*cast)(void* object) = nullptr;
	};

	// Everything the program knows about a type while it runs.
	class TypeInfo
	{
	public:
		using CreateFunction = void* (*)();

		TypeInfo(std::string_view name, std::uint32_t version, std::vector<BaseInfo> bases, CreateFunction create);

		// Each type has exactly one TypeInfo: copying it would break that.
		TypeInfo(const TypeInfo&) = delete;
		TypeInfo& operator=(const TypeInfo&) = delete;

		std::string_view GetName() const { return m_name; }
		TypeId GetId() const { return m_id; }
		std::uint32_t GetVersion() const { return m_version; }
		std::span<const BaseInfo> GetBases() const { return m_bases; }

		// True if this type is target, or inherits from it, directly or not.
		bool IsA(const TypeInfo& target) const;

		// Turns a pointer to an object of this type into a pointer to target, or returns nullptr.
		void* CastTo(void* object, const TypeInfo& target) const;

		// Abstract types, and types without a default constructor, cannot be created.
		bool CanCreate() const { return m_create != nullptr; }
		void* Create() const;

	private:
		std::string_view m_name;
		TypeId m_id;
		std::uint32_t m_version = 0;
		std::vector<BaseInfo> m_bases;
		CreateFunction m_create = nullptr;
	};
}

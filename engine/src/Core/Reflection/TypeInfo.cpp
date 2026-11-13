#include <AssetEngine/Core/Reflection/TypeInfo.h>

#include <utility>

namespace AssetEngine
{
	TypeInfo::TypeInfo(std::string_view name, std::uint32_t version, std::vector<BaseInfo> bases, CreateFunction create)
		: m_name(name)
		, m_id(TypeIdFromName(name))
		, m_version(version)
		, m_bases(std::move(bases))
		, m_create(create)
	{
	}

	bool TypeInfo::IsA(const TypeInfo& target) const
	{
		if (m_id == target.m_id)
			return true;

		for (const BaseInfo& base : m_bases)
		{
			if (base.type->IsA(target))
				return true;
		}

		return false;
	}

	void* TypeInfo::CastTo(void* object, const TypeInfo& target) const
	{
		if (object == nullptr)
			return nullptr;

		if (m_id == target.m_id)
			return object;

		// Climb each base in turn, adjusting the pointer at every step.
		for (const BaseInfo& base : m_bases)
		{
			if (void* result = base.type->CastTo(base.cast(object), target))
				return result;
		}

		return nullptr;
	}

	void* TypeInfo::Create() const
	{
		return m_create != nullptr ? m_create() : nullptr;
	}
}

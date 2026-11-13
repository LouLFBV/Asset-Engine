#include <AssetEngine/Core/Reflection/TypeRegistry.h>

namespace AssetEngine
{
	TypeRegistry& TypeRegistry::Global()
	{
		// Created the first time it is used, so it already exists when the first type registers.
		static TypeRegistry registry;
		return registry;
	}

	bool TypeRegistry::Register(const TypeInfo& type)
	{
		if (m_frozen)
			return false;

		const auto [entry, inserted] = m_types.emplace(type.GetId(), &type);

		// Registering the same type twice is harmless. Two types with the same id is an error.
		return inserted || entry->second == &type;
	}

	const TypeInfo* TypeRegistry::Find(TypeId id) const
	{
		const auto entry = m_types.find(id);
		return entry != m_types.end() ? entry->second : nullptr;
	}
}

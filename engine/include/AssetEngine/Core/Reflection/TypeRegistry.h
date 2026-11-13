#pragma once

#include <AssetEngine/Core/Reflection/Reflect.h>
#include <AssetEngine/Core/Reflection/TypeId.h>
#include <AssetEngine/Core/Reflection/TypeInfo.h>

#include <cstddef>
#include <string_view>
#include <unordered_map>

namespace AssetEngine
{
	// The list of the types the program can find by id: filled before main, then frozen.
	class TypeRegistry
	{
	public:
		// The registry of the whole program.
		static TypeRegistry& Global();

		// Returns false if the registry is frozen, or if another type already has this id.
		bool Register(const TypeInfo& type);

		// After this call, no type can be added: the registry can be read from any thread.
		void Freeze() { m_frozen = true; }
		bool IsFrozen() const { return m_frozen; }

		const TypeInfo* Find(TypeId id) const;
		const TypeInfo* Find(std::string_view name) const { return Find(TypeIdFromName(name)); }
		std::size_t Count() const { return m_types.size(); }

	private:
		std::unordered_map<TypeId, const TypeInfo*> m_types;
		bool m_frozen = false;
	};
}

#define ASSET_CONCAT_IMPL(a, b) a##b
#define ASSET_CONCAT(a, b) ASSET_CONCAT_IMPL(a, b)

// Registers a reflected type in the global registry, before main starts.
// Write it once, in a .cpp file, at namespace scope.
#define ASSET_REGISTER_TYPE(Type)                                                                          \
	namespace                                                                                              \
	{                                                                                                      \
		const bool ASSET_CONCAT(assetEngineRegistered, __LINE__) =                                         \
			::AssetEngine::TypeRegistry::Global().Register(::AssetEngine::TypeOf<Type>());                 \
	}

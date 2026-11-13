#pragma once

#include <AssetEngine/Core/Reflection/Reflect.h>
#include <AssetEngine/Core/Reflection/TypeId.h>
#include <AssetEngine/Core/Reflection/TypeRegistry.h>

#include <memory>
#include <type_traits>

namespace AssetEngine
{
	class Factory
	{
	public:
		// Creates the type that has this id, and returns it as a Base.
		// Returns nullptr if the type is unknown, cannot be created, or does not inherit from Base.
		template <Reflected Base>
		static std::unique_ptr<Base> Create(TypeId id, const TypeRegistry& registry = TypeRegistry::Global())
		{
			static_assert(std::has_virtual_destructor_v<Base>, "Base needs a virtual destructor to be deleted through a Base pointer");

			const TypeInfo* type = registry.Find(id);
			if (type == nullptr || !type->CanCreate() || !type->IsA(TypeOf<Base>()))
				return nullptr;

			// The object is created as its real type, then the pointer is moved to the Base part.
			void* object = type->Create();
			return std::unique_ptr<Base>(static_cast<Base*>(type->CastTo(object, TypeOf<Base>())));
		}
	};
}

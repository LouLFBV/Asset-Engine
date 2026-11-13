#pragma once

#include <AssetEngine/Core/Reflection/TypeId.h>
#include <AssetEngine/Core/Reflection/TypeInfo.h>

#include <concepts>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <vector>

namespace AssetEngine
{
	// A list of types, used to declare the bases of a type.
	template <typename... Types>
	struct TypeList
	{
	};

	// A type that declares its name, its version and its bases.
	template <typename T>
	concept Reflected = Named<T> && requires
	{
		{ T::TypeVersion } -> std::convertible_to<std::uint32_t>;
		typename T::TypeBases;
	};

	template <Reflected T>
	const TypeInfo& TypeOf();

	namespace Detail
	{
		template <typename T, typename... Bases>
		std::vector<BaseInfo> MakeBases(TypeList<Bases...>)
		{
			static_assert((std::is_base_of_v<Bases, T> && ...), "TypeBases must only list base classes of the type");

			// static_cast knows the real offset of each base, even with multiple or virtual inheritance.
			return {BaseInfo{&TypeOf<Bases>(), [](void* object) -> void* {
				return static_cast<Bases*>(static_cast<T*>(object));
			}}...};
		}

		template <typename T>
		constexpr TypeInfo::CreateFunction CreateFunctionOf()
		{
			if constexpr (!std::is_abstract_v<T> && std::is_default_constructible_v<T>)
				return []() -> void* { return new T(); };
			else
				return nullptr;
		}
	}

	// The TypeInfo of T, built once, the first time it is asked for.
	template <Reflected T>
	const TypeInfo& TypeOf()
	{
		static const TypeInfo info(T::TypeName, T::TypeVersion, Detail::MakeBases<T>(typename T::TypeBases{}), Detail::CreateFunctionOf<T>());
		return info;
	}
}

// Declares the reflection of a class: its name, its version, then its bases, if any.
// Put it at the top of the class. The members that follow it are public.
#define ASSET_REFLECT(Type, Version, ...)                                                   \
public:                                                                                     \
	static constexpr std::string_view TypeName = #Type;                                     \
	static constexpr std::uint32_t TypeVersion = Version;                                   \
	using TypeBases = ::AssetEngine::TypeList<__VA_ARGS__>;                                 \
	virtual const ::AssetEngine::TypeInfo& GetTypeInfo() const                              \
	{                                                                                       \
		return ::AssetEngine::TypeOf<Type>();                                               \
	}

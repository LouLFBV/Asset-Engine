#pragma once

#include <AssetEngine/Core/Hash.h>
#include <compare>
#include <cstdint>
#include <string_view>
#include <cstddef>
#include <functional>
#include <concepts>

namespace AssetEngine
{
    // A type identifier, the hash of a type name.
    struct TypeId
    {
        std::uint64_t value = 0;

        constexpr auto operator<=>(const TypeId&) const = default;
    };

    constexpr TypeId TypeIdFromName(std::string_view name)
    {
        return TypeId{Fnv1a64(name)};
    }

    //A type that declares its name as a static constant.
    template <typename T>
    concept Named = requires
    {
        {T::TypeName } -> std::convertible_to<std::string_view>;
    };

    // The id of a type, computed by the compiler
    template <Named T>
    inline constexpr TypeId TypeIdOf = TypeIdFromName(T::TypeName);
}

//Lets TypeId be the key of a std::unordered_map
template <>
struct std::hash<AssetEngine::TypeId>
{
    std::size_t operator()(AssetEngine::TypeId id) const noexcept
    {
        return static_cast<std::size_t>(id.value);
    }
};
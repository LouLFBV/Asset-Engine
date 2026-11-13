#include <AssetEngine/Core/Reflection/Reflect.h>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <string>

using namespace AssetEngine;

namespace
{
	// Two unrelated bases, then a class that inherits from both.
	class Entity
	{
		ASSET_REFLECT(Entity, 1)

		virtual ~Entity() = default;
		std::string name = "entity";
	};

	class Damageable
	{
		ASSET_REFLECT(Damageable, 2)

		virtual ~Damageable() = default;
		std::int32_t hp = 100;
	};

	class Knight : public Entity, public Damageable
	{
		ASSET_REFLECT(Knight, 3, Entity, Damageable)
	};

	// A diamond: Joint reaches Node through Left and through Right, but holds only one Node.
	class Node
	{
		ASSET_REFLECT(Node, 1)

		virtual ~Node() = default;
	};

	class Left : public virtual Node
	{
		ASSET_REFLECT(Left, 1, Node)
	};

	class Right : public virtual Node
	{
		ASSET_REFLECT(Right, 1, Node)
	};

	class Joint : public Left, public Right
	{
		ASSET_REFLECT(Joint, 1, Left, Right)
	};

	class Shape
	{
		ASSET_REFLECT(Shape, 1)

		virtual ~Shape() = default;
		virtual float Area() const = 0;
	};

	// The static parameters are known without creating any object.
	static_assert(Knight::TypeName == "Knight");
	static_assert(Knight::TypeVersion == 3);
	static_assert(Reflected<Knight>);
}

TEST_CASE("TypeInfo describes a type without any instance")
{
	const TypeInfo& knight = TypeOf<Knight>();

	REQUIRE(knight.GetName() == "Knight");
	REQUIRE(knight.GetVersion() == 3);
	REQUIRE(knight.GetId() == TypeIdOf<Knight>);
	REQUIRE(knight.GetBases().size() == 2);
	REQUIRE(knight.GetBases()[0].type == &TypeOf<Entity>());
	REQUIRE(knight.GetBases()[1].type == &TypeOf<Damageable>());

	// Asking twice gives the very same TypeInfo.
	REQUIRE(&TypeOf<Knight>() == &knight);
}

TEST_CASE("TypeInfo knows the whole inheritance tree")
{
	REQUIRE(TypeOf<Knight>().IsA(TypeOf<Knight>()));
	REQUIRE(TypeOf<Knight>().IsA(TypeOf<Entity>()));
	REQUIRE(TypeOf<Knight>().IsA(TypeOf<Damageable>()));
	REQUIRE_FALSE(TypeOf<Entity>().IsA(TypeOf<Knight>()));
	REQUIRE_FALSE(TypeOf<Entity>().IsA(TypeOf<Damageable>()));
}

TEST_CASE("TypeInfo moves the pointer to the second base")
{
	Knight knight;
	void* asDamageable = TypeOf<Knight>().CastTo(&knight, TypeOf<Damageable>());

	// The second base does not start at the address of the object: the pointer must move.
	REQUIRE(asDamageable == static_cast<Damageable*>(&knight));
	REQUIRE(asDamageable != static_cast<void*>(&knight));
	REQUIRE(static_cast<Damageable*>(asDamageable)->hp == 100);

	REQUIRE(TypeOf<Entity>().CastTo(&knight, TypeOf<Damageable>()) == nullptr);
}

TEST_CASE("The real type is found through any base")
{
	Knight knight;
	const Entity& asEntity = knight;
	const Damageable& asDamageable = knight;

	REQUIRE(asEntity.GetTypeInfo().GetName() == "Knight");
	REQUIRE(asDamageable.GetTypeInfo().GetName() == "Knight");
}

TEST_CASE("TypeInfo handles a diamond with a virtual base")
{
	Joint joint;

	REQUIRE(TypeOf<Joint>().IsA(TypeOf<Node>()));
	REQUIRE(TypeOf<Joint>().CastTo(&joint, TypeOf<Node>()) == static_cast<Node*>(&joint));

	// Through Left or through Right, both paths lead to the same, single Node.
	Left& asLeft = joint;
	Right& asRight = joint;
	REQUIRE(static_cast<Node*>(&asLeft) == static_cast<Node*>(&asRight));
}

TEST_CASE("TypeInfo knows which types can be created")
{
	REQUIRE(TypeOf<Knight>().CanCreate());
	REQUIRE_FALSE(TypeOf<Shape>().CanCreate());
	REQUIRE(TypeOf<Shape>().Create() == nullptr);
}

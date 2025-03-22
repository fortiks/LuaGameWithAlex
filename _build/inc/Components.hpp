#pragma once

#include "lua.hpp"
#include "raylib.h"

#include <cstring>

enum Materials
{
	UNBREAKABLE = 0,
	SURFACE_GRASS,
	FINAL_LEVEL_WALL,
	FINAL_LEVEL_GROUND,
	EMPTY,
	GRASS,
	DIRT,
	STONE,
	BRONZE,
	SILVER,
	GOLDEN,
	DIAMOND,
	ANCIENT_RELIC
};

struct Gravitation
{
	float VelocityY;
	float GravityFactor;
	bool IsTouchingGround;
};

struct Health
{
	float CurrentHealth;
	float MaxHealth;
};

struct Fuel
{
	float CurrentFuel;
	float MaxFuel;
};

struct LUAVector3
{
	float X, Y, Z;
	LUAVector3(float x = 0.f, float y = 0.f, float z = 0.f) :
		X(x), Y(y), Z(z) {}
};

struct LUATransform
{
	LUAVector3 Position;
	LUAVector3 Rotation;
	LUAVector3 Scale;
	LUATransform(LUAVector3 position = LUAVector3(),
		LUAVector3 rotation = LUAVector3(),
		LUAVector3 scale = LUAVector3()) :
		Position(position),
		Rotation(rotation),
		Scale(scale) {}
};

struct Backpack
{
	float MaxWeight;
	float CurrentWeight;
	int DirtCount;
	int StoneCount;
	int BronzeCount;
	int SilverCount;
	int GoldCount;
	int DiamondCount;
	int AncientRelicCount;
};

struct Character
{
	float Gold;
	float Damage;
	LUATransform Transform;
};

struct Treasure
{
	float Gold;
	float Durability;
	float MaxDurability;
	LUATransform Transform;
};

struct Mole
{
	float Health;
	float MaxHealth;
	float Damage;
	LUATransform Transform;
	bool IsStunned;
};

struct Behaviour
{
	char Path[128];
	int LuaRef;

	// Create a constructor in order to initialize the char array.
	Behaviour(const char* path, int luaRef) : LuaRef(luaRef)
	{
		memset(Path, '\0', 128);
		strcpy_s(Path, path);
	}
};

struct Shop
{
	int Id;
	char Name[64];
	float ScreenPointX;
	float ScreenPointY;
	float ScreenWidth;
	float ScreenHeight;
	bool ShopOpen;
	LUATransform Transform;
	LUAVector3 Color;
	int OnOpenRef;

	Shop(int id, const char* name, float screenPointX, float screenPointY, float screenWidth, float screenHeight, bool shopOpen, const LUATransform& transform, const LUAVector3& color, int onOpenRef)
	{
		Id = id;
		memset(Name, '\0', 64);
		strcpy_s(Name, name);
		ScreenPointX = screenPointX;
		ScreenPointY = screenPointY;
		ScreenWidth = screenWidth;
		ScreenHeight = screenHeight;
		ShopOpen = shopOpen;
		Transform = transform;
		Color = color;
		OnOpenRef = onOpenRef;
	}
};

struct Button
{
	char Name[64];
	int ShopId;
	int Tooltip;
	float ScreenPointX;
	float ScreenPointY;
	float ScreenWidth;
	float ScreenHeight;
	int OnClickRef;

	Button(const char* name, int shopId, int tooltip, float screenPointX, float screenPointY, float screenWidth, float screenHeight, int onClickRef)
	{
		memset(Name, '\0', 64);
		strcpy_s(Name, name);
		ShopId = shopId;
		Tooltip = tooltip;
		ScreenPointX = screenPointX;
		ScreenPointY = screenPointY;
		ScreenWidth = screenWidth;
		ScreenHeight = screenHeight;
		OnClickRef = onClickRef;
	}
};

struct TextLabel
{
	char Name[64];
	int Id;
	int ShopId;
	float ScreenPointX;
	float ScreenPointY;
	float ScreenWidth;
	float ScreenHeight;

	TextLabel(const char* name, int id, int shopId, float screenPointX, float screenPointY, float screenWidth, float screenHeight)
	{
		memset(Name, '\0', 64);
		strcpy_s(Name, name);
		Id = id;
		ShopId = shopId;
		ScreenPointX = screenPointX;
		ScreenPointY = screenPointY;
		ScreenWidth = screenWidth;
		ScreenHeight = screenHeight;
	}
};

struct Block
{
	int Gold;
	int Weight;
	LUAVector3 Position;
	LUAVector3 Size;
	Materials Material;
	Block(int gold, int weight, const LUAVector3& position, const LUAVector3& size, Materials material)
		: Gold(gold), Weight(weight), Position(position), Size(size), Material(material)
	{
	}
};

struct GUIRec
{
	float PosX;
	float PosY;
	float SizeX;
	float SizeY;
	LUAVector3 Color;
};

struct GUITextLabel
{
	float PosX;
	float PosY;
	char Text[64];

	GUITextLabel(float posX, float posY,  const char* text)
		: PosX(posX), PosY(posY)
	{
		memset(Text, '\0', 64);
		strcpy_s(Text, text);
	}
};

struct GUIButton
{
	int X;
	int Y;
	LUAVector3 OnClickRef;
};

void DumpError(lua_State* L);
void DumpStack(lua_State* L);

// Pops a ’vector ’ table from the stack and returns an instance
// the the type Vector .
LUAVector3 lua_tovector(lua_State* L, int index);
/*
Lua arguments in table :
	1 | table | -1
	1: a table representing the vector

Return values : none
*/
static int PrintVector(lua_State* L);
// Pushes a 'vector' table to the stack , based on
// values from a given Vector instance .
void lua_pushvector(lua_State* L, const LUAVector3& vector);
/*
Lua arguments in table :
	2 | number | -1
	1 | number | -2
1: floor of the randomized values
2: roof of the randomize values

Return values :
	A table representing the
	randomized vector .
 */
static int RandomVector(lua_State* L);
// Pops a ’transform ’ table from the stack and returns an instance
// the the type Transform .
LUATransform lua_totransform(lua_State* L, int index);
// Pushes a 'transform' table to the stack , based on
// values from a given Transform instance .
void lua_pushtransform(lua_State* L, const LUATransform& transform);
/**
* Prints a ’transform ’ table .
* Input in stack :
* 1 | table the ’transform ’ table to print
*/
static int PrintTransform(lua_State* L);
/**
* Creates and returns a new ’transform ’ table with
* all x, y, z fields randomized between given floor
* and roof values .
*
* Input in stack :
* 2 | number roof for randomized values
* 1 | number floor for randomized values
*/
static int RandomTransform(lua_State* L);
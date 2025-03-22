#include "Components.hpp"

#include <iostream>
#include <string>
#include <iomanip>

void DumpError(lua_State* L)
{
	if (lua_gettop(L) && lua_isstring(L, -1))
	{
		std::cout << "Lua error: " << lua_tostring(L, -1) << std::endl;
		lua_pop(L, 1);
	}
}

void DumpStack(lua_State* L)
{
	int size = lua_gettop(L);

	std::cout << " " << std::string(11, '-');
	std::cout << " STACK BEGIN ";
	std::cout << std::string(11, '-') << std::endl;

	for (int i = size; i > 0; i--)
	{
		int type = lua_type(L, i);
		std::string typeName = lua_typename(L, type);
		std::string value;

		if (type == LUA_TSTRING)
		{
			value = "\"";
			value += lua_tostring(L, i);

			if (value.size() > 11)
			{
				value.resize(9);
				value += "..";
			}

			value += "\"";
		}
		else if (type == LUA_TBOOLEAN)
		{
			value = lua_toboolean(L, i) ? "true" : "false";
		}
		else if (type == LUA_TNIL)
		{
			value = "nil";
		}
		else if (lua_isnumber(L, i))
		{
			value = std::to_string(lua_tonumber(L, i));
		}
		else if (lua_isstring(L, i))
		{
			value = lua_tostring(L, i);
		}

		std::cout << std::setw(3) << i << " | ";
		std::cout << typeName << std::setw(25 - typeName.size()) << value;
		// std::cout << std::setw(5 - typeName.size() - value.size()) << " | ";
		std::cout << std::setw(2) << -(size - i + 1);
		std::cout << std::endl;
	}

	std::cout << " " << std::string(12, '-');
	std::cout << "STACK END";
	std::cout << std::string(12, '-') << std::endl;
}

LUAVector3 lua_tovector(lua_State* L, int index)
{
	if (!lua_istable(L, index))
	{
		std::cout << "table expected" << std::endl;
	}

	lua_getfield(L, index, "x");
	float x = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "y");
	float y = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, index, "z");
	float z = lua_tonumber(L, -1);
	lua_pop(L, 1);

	return { x, y, z };
}

/*
Lua arguments in table :
	1 | table | -1
	1: a table representing the vector

Return values : none
*/
static int PrintVector(lua_State* L)
{
	LUAVector3 vector = lua_tovector(L, 1);

	std::cout << "[C++] Vector ("
		<< vector.X << ", "
		<< vector.Y << ", "
		<< vector.Z << ")"
		<< std::endl;

	// No return values
	return 0;
}

void lua_pushvector(lua_State* L, const LUAVector3& vector)
{
	lua_newtable(L); // Push new vector table

	lua_pushnumber(L, vector.X);
	lua_setfield(L, -2, "x");

	lua_pushnumber(L, vector.Y);
	lua_setfield(L, -2, "y");

	lua_pushnumber(L, vector.Z);
	lua_setfield(L, -2, "z");
}

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
static int RandomVector(lua_State* L)
{
	// Sanity check
	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
		return 0;

	int min = lua_tonumber(L, 1);
	int max = lua_tonumber(L, 2);
	int diff = max - min;
	lua_pop(L, 2);

	LUAVector3 vector(rand() % diff + min,
		rand() % diff + min,
		rand() % diff + min);

	lua_pushvector(L, vector);
	return 1;
}

// Pops a ’transform ’ table from the stack and returns an instance
// the the type Transform .
LUATransform lua_totransform(lua_State* L, int index)
{
	// Sanity check
	if (!lua_istable(L, index))
		throw " lua_totransform : table expected .";

	LUATransform transform;
	lua_getfield(L, index, "position");
	transform.Position = lua_tovector(L, -1);
	lua_getfield(L, index, "rotation");
	transform.Rotation = lua_tovector(L, -1);
	lua_getfield(L, index, "scale");
	transform.Scale = lua_tovector(L, -1);

	return transform;
}

// Pushes an ’transform ’ table to the stack , based on
// values from a given Transform instance .
void lua_pushtransform(lua_State* L, const LUATransform& transform)
{
	lua_newtable(L); // Push transform table

	lua_pushvector(L, transform.Position);
	lua_setfield(L, -2, "position");
	lua_pushvector(L, transform.Rotation);
	lua_setfield(L, -2, "rotation");
	lua_pushvector(L, transform.Scale);
	lua_setfield(L, -2, "scale");
}

/**
* Prints a ’transform ’ table .
* Input in stack :
* 1 | table the ’transform ’ table to print
*/
static int PrintTransform(lua_State* L)
{
	LUATransform transform = lua_totransform(L, 1);

	std::cout << " Transform :" << std::endl;

	std::cout << " Position ("
		<< transform.Position.X << ", "
		<< transform.Position.Y << ", "
		<< transform.Position.Z << ")"
		<< std::endl;

	std::cout << " Rotation ("
		<< transform.Position.X << ", "
		<< transform.Position.Y << ", "
		<< transform.Position.Z << ")"
		<< std::endl;

	std::cout << " Scale ("
		<< transform.Position.X << ", "
		<< transform.Position.Y << ", "
		<< transform.Position.Z << ")"
		<< std::endl;

	return 0;
}

/**
* Creates and returns a new ’transform ’ table with
* all x, y, z fields randomized between given floor
* and roof values .
*
* Input in stack :
* 2 | number roof for randomized values
* 1 | number floor for randomized values
*/
static int RandomTransform(lua_State* L)
{
	lua_newtable(L); // Push an empty table

	lua_pushvalue(L, 1); // Push copy of argument
	lua_pushvalue(L, 2); // Push copy of argument
	RandomVector(L); // Push random position
	lua_setfield(L, -2, " position ");

	lua_pushvalue(L, 1); // Push copy of argument
	lua_pushvalue(L, 2); // Push copy of argument
	RandomVector(L); // Push random rotation
	lua_setfield(L, -2, " rotation ");

	lua_pushvalue(L, 1); // Push copy of argument
	lua_pushvalue(L, 2); // Push copy of argument
	RandomVector(L); // Push random scale
	lua_setfield(L, -2, " scale ");

	return 1;
}
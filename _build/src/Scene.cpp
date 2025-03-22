#include "Scene.hpp"
#include <fstream>
#include <cstdio>
#include <filesystem>

constexpr const int SCREEN_WIDTH = 1768;
constexpr const int SCREEN_HEIGHT = 992;

Scene::Scene(lua_State* L, std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> blockEntities,
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>> staticBlockEntities, std::shared_ptr<Camera3D> camera)
	: m_luaState(L), m_BlockEntities(blockEntities), m_StaticBlockEntities(staticBlockEntities), m_Camera(camera)
{
}

int Scene::GetEntityCount()
{
	return static_cast<int>(m_registry.alive());
}

int Scene::CreateEntity()
{
	return static_cast<int>(m_registry.create());
}

bool Scene::IsEntity(int entity)
{
	return m_registry.valid(static_cast<entt::entity>(entity));
}

void Scene::RemoveEntity(int entity)
{
	m_registry.destroy(static_cast<entt::entity>(entity));
}

void Scene::UpdateSystems(float delta)
{
	for (auto it = m_systems.begin(); it != m_systems.end();)
	{
		if ((*it)->OnUpdate(m_registry, delta))
		{
			delete (*it);
			it = m_systems.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void Scene::lua_openscene(lua_State* L, Scene* scene)
{
	lua_newtable(L);
	luaL_Reg sceneMethods[] =
	{
		{ "GetEntityCount", lua_GetEntityCount },
		{ "GetEntities", lua_GetEntities },
		{ "CreateEntity", lua_CreateEntity },
		{ "IsEntity", lua_IsEntity },
		{ "RemoveEntity", lua_RemoveEntity },
		{ "HasComponent", lua_HasComponent },
		{ "GetComponent", lua_GetComponent },
		{ "SetComponent", lua_SetComponent },
		{ "RemoveComponent", lua_RemoveComponent },
		{ "IsKeyDown", lua_IsKeyDown },
		{ "IsKeyPressed", lua_IsKeyPressed },
		{ "IsMouseButtonPressed", lua_IsMouseButtonPressed },
		{ "GetEntityAndComponentFromClick", lua_GetEntityAndComponentFromClick },
		{ "SaveGame", lua_SaveGame },
		{ "LoadGame", lua_LoadGame },
		{ "SaveTables", lua_SaveTables },
		{ "LoadTables", lua_LoadTables },
		{ "DeleteFiles", lua_DeleteFiles },
		{ "GetRayCenter", lua_GetRayCenter },
		{ "GetRayCollisionBox", lua_GetRayCollisionBox },
		{ NULL, NULL }
	};
	lua_pushlightuserdata(L, scene);
	luaL_setfuncs(L, sceneMethods, 1);
	lua_setglobal(L, "scene");
}

Scene* Scene::lua_GetSceneUpValue(lua_State* L)
{
	Scene* scene = nullptr;
	if (lua_isuserdata(L, lua_upvalueindex(1)))
	{
		scene = (Scene*)lua_touserdata(L, lua_upvalueindex(1));
	}
	return scene;
}

int Scene::lua_GetEntityCount(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int count = scene->GetEntityCount();
	lua_pushinteger(L, count);
	return 1;
}

int Scene::lua_CreateEntity(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int entity = scene->CreateEntity();
	lua_pushinteger(L, entity);
	return 1;
}

int Scene::lua_IsEntity(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	bool alive = scene->IsEntity(entity);
	lua_pushboolean(L, alive);
	return 1;
}

int Scene::lua_RemoveEntity(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	scene->RemoveEntity(entity);
	return 0;
}

int Scene::lua_HasComponent(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);
	bool hasComponent = true; // false

	if (type == "health") {
		hasComponent = scene->HasComponents<Health>(entity);
	}
	else if (type == "backpack") {
		hasComponent = scene->HasComponents<Backpack>(entity);
	}
	else if (type == "gravitation") {
		hasComponent = scene->HasComponents<Gravitation>(entity);
	}
	else if (type == "gravitation") {
		hasComponent = scene->HasComponents<Gravitation>(entity);
	}
	else if (type == "transform") {
		hasComponent = scene->HasComponents<LUATransform>(entity);
	}
	else if (type == "block") {
		hasComponent = scene->HasComponents<Block>(entity);
	}
	else if (type == "mole") {
		hasComponent = scene->HasComponents<Mole>(entity);
	}
	else if (type == "treasure") {
		hasComponent = scene->HasComponents<Treasure>(entity);
	}
	else if (type == "behaviour") {
		hasComponent = scene->HasComponents<Behaviour>(entity);
	}
	else if (type == "textLabel") {
		hasComponent = scene->HasComponents<TextLabel>(entity);
	}

	lua_pushboolean(L, hasComponent);
	return 1;
}

int Scene::lua_GetComponent(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);

	if (!lua_isinteger(L, 1) || !lua_isstring(L, 2))
	{
		lua_pushnil(L);
		return 1;
	}

	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);

	// Sanity check that the entity exist
	if (!scene->IsEntity(entity))
	{
		lua_pushnil(L);
		return 1;
	}

	if (type == "character" && scene->HasComponents<Character>(entity))
	{
		Character& character = scene->GetComponent<Character>(entity);
		lua_newtable(L);
		lua_pushnumber(L, character.Gold);
		lua_setfield(L, -2, "gold");
		lua_pushnumber(L, character.Damage);
		lua_setfield(L, -2, "damage");
		lua_pushtransform(L, character.Transform);
		lua_setfield(L, -2, "transform");
		return 1;
	}
	else if (type == "backpack" && scene->HasComponents<Backpack>(entity))
	{
		Backpack& backpack = scene->GetComponent<Backpack>(entity);
		lua_newtable(L);
		lua_pushnumber(L, backpack.MaxWeight);
		lua_setfield(L, -2, "maxWeight");
		lua_pushnumber(L, backpack.CurrentWeight);
		lua_setfield(L, -2, "currentWeight");
		lua_pushnumber(L, backpack.DirtCount);
		lua_setfield(L, -2, "dirtCount");
		lua_pushnumber(L, backpack.StoneCount);
		lua_setfield(L, -2, "stoneCount");
		lua_pushnumber(L, backpack.BronzeCount);
		lua_setfield(L, -2, "bronzeCount");
		lua_pushnumber(L, backpack.SilverCount);
		lua_setfield(L, -2, "silverCount");
		lua_pushnumber(L, backpack.GoldCount);
		lua_setfield(L, -2, "goldenCount");
		lua_pushnumber(L, backpack.DiamondCount);
		lua_setfield(L, -2, "diamondCount");
		lua_pushnumber(L, backpack.AncientRelicCount);
		lua_setfield(L, -2, "ancientRelicCount");
		return 1;
	}
	else if (type == "mole" && scene->HasComponents<Mole>(entity))
	{
		Mole& mole = scene->GetComponent<Mole>(entity);
		lua_newtable(L);
		lua_pushnumber(L, mole.Health);
		lua_setfield(L, -2, "health");
		lua_pushnumber(L, mole.MaxHealth);
		lua_setfield(L, -2, "maxHealth");
		lua_pushnumber(L, mole.Damage);
		lua_setfield(L, -2, "damage");
		lua_pushtransform(L, mole.Transform);
		lua_setfield(L, -2, "transform");
		lua_pushboolean(L, mole.IsStunned);
		lua_setfield(L, -2, "isStunned");
		return 1;
	}
	else if (type == "shop" && scene->HasComponents<Shop>(entity))
	{
		Shop shop = scene->GetComponent<Shop>(entity);
		lua_newtable(L);
		lua_pushinteger(L, shop.Id);
		lua_setfield(L, -2, "id");
		lua_pushstring(L, shop.Name);
		lua_setfield(L, -2, "name");
		lua_pushnumber(L, shop.ScreenPointX);
		lua_setfield(L, -2, "screenPointX");
		lua_pushnumber(L, shop.ScreenPointY);
		lua_setfield(L, -2, "screenPointY");
		lua_pushnumber(L, shop.ScreenWidth);
		lua_setfield(L, -2, "screenWidth");
		lua_pushnumber(L, shop.ScreenHeight);
		lua_setfield(L, -2, "screenHeight");
		lua_pushboolean(L, shop.ShopOpen);
		lua_setfield(L, -2, "shopOpen");
		lua_pushtransform(L, shop.Transform);
		lua_setfield(L, -2, "transform");
		lua_pushvector(L, shop.Color);
		lua_setfield(L, -2, "color");
		lua_pushinteger(L, shop.OnOpenRef);
		lua_setfield(L, -2, "onOpenRef");

		return 1;
	}
	/*else if (type == "button" && scene->HasComponents<Button>(entity))
	{
		Button button = scene->GetComponent<Button>(entity);
		lua_newtable(L);
		lua_pushstring(L, button.Name);
		lua_setfield(L, -2, "name");
		lua_pushinteger(L, button.ShopId);
		lua_setfield(L, -2, "shopId");
		lua_pushnumber(L, button.ScreenPointX);
		lua_setfield(L, -2, "screenPointX");
		lua_pushnumber(L, button.ScreenPointY);
		lua_setfield(L, -2, "screenPointY");
		lua_pushnumber(L, button.ScreenWidth);
		lua_setfield(L, -2, "screenWidth");
		lua_pushnumber(L, button.ScreenHeight);
		lua_setfield(L, -2, "screenHeight");

		return 1;
	}*/
	else if (type == "textLabel" && scene->HasComponents<TextLabel>(entity))
	{
		TextLabel textLabel = scene->GetComponent<TextLabel>(entity);
		lua_newtable(L);
		lua_pushstring(L, textLabel.Name);
		lua_setfield(L, -2, "name");
		lua_pushinteger(L, textLabel.Id);
		lua_setfield(L, -2, "id");
		lua_pushinteger(L, textLabel.ShopId);
		lua_setfield(L, -2, "shopId");
		lua_pushnumber(L, textLabel.ScreenPointX);
		lua_setfield(L, -2, "screenPointX");
		lua_pushnumber(L, textLabel.ScreenPointY);
		lua_setfield(L, -2, "screenPointY");
		lua_pushnumber(L, textLabel.ScreenWidth);
		lua_setfield(L, -2, "screenWidth");
		lua_pushnumber(L, textLabel.ScreenHeight);
		lua_setfield(L, -2, "screenHeight");

		return 1;
	}
	else if (type == "treasure" && scene->HasComponents<Treasure>(entity))
	{
		Treasure& treasure = scene->GetComponent<Treasure>(entity);
		lua_newtable(L);
		lua_pushnumber(L, treasure.Gold);
		lua_setfield(L, -2, "gold");
		lua_pushnumber(L, treasure.Durability);
		lua_setfield(L, -2, "durability");
		lua_pushnumber(L, treasure.MaxDurability);
		lua_setfield(L, -2, "maxDurability");
		lua_pushtransform(L, treasure.Transform);
		lua_setfield(L, -2, "transform");
		return 1;
	}
	else if (type == "health" && scene->HasComponents<Health>(entity))
	{
		Health& health = scene->GetComponent<Health>(entity);
		lua_pushnumber(L, health.CurrentHealth);
		lua_pushnumber(L, health.MaxHealth);
		return 2;
	}
	else if (type == "fuel" && scene->HasComponents<Fuel>(entity))
	{
		Fuel& fuel = scene->GetComponent<Fuel>(entity);
		lua_pushnumber(L, fuel.CurrentFuel);
		lua_pushnumber(L, fuel.MaxFuel);
		return 2;
	}
	else if (type == "gravitation" && scene->HasComponents<Gravitation>(entity))
	{
		Gravitation& gravitation = scene->GetComponent<Gravitation>(entity);
		lua_newtable(L);
		lua_pushnumber(L, gravitation.VelocityY);
		lua_setfield(L, -2, "velocityY");
		lua_pushnumber(L, gravitation.GravityFactor);
		lua_setfield(L, -2, "gravityFactor");
		lua_pushboolean(L, gravitation.IsTouchingGround);
		lua_setfield(L, -2, "isTouchingGround");
		return 1;
	}
	else if (type == "block" && scene->HasComponents<Block>(entity))
	{
		Block& block = scene->GetComponent<Block>(entity);
		lua_newtable(L);
		lua_pushinteger(L, block.Gold);
		lua_setfield(L, -2, "gold");
		lua_pushinteger(L, block.Weight);
		lua_setfield(L, -2, "weight");
		lua_pushvector(L, block.Position);
		lua_setfield(L, -2, "position");
		lua_pushvector(L, block.Size);
		lua_setfield(L, -2, "size");
		lua_pushinteger(L, block.Material);
		lua_setfield(L, -2, "material");
		return 1;
	}
	else if (type == "transform" && scene->HasComponents<LUATransform>(entity))
	{
		LUATransform& transform = scene->GetComponent<LUATransform>(entity);
		lua_pushtransform(L, transform); // from course module M2 -M3
		return 1;
	}
	else if (type == "guiRec" && scene->HasComponents<GUIRec>(entity))
	{
		GUIRec& guiRec = scene->GetComponent<GUIRec>(entity);
		lua_newtable(L);
		lua_pushnumber(L, guiRec.PosX);
		lua_setfield(L, -2, "posX");
		lua_pushnumber(L, guiRec.PosY);
		lua_setfield(L, -2, "posY");
		lua_pushnumber(L, guiRec.SizeX);
		lua_setfield(L, -2, "sizeX");
		lua_pushnumber(L, guiRec.SizeY);
		lua_setfield(L, -2, "sizeY");
		lua_pushvector(L, guiRec.Color);
		lua_setfield(L, -2, "color");
		return 1;
	}
	else if (type == "guiTextLabel" && scene->HasComponents<GUITextLabel>(entity))
	{
		GUITextLabel& guiTextLabel = scene->GetComponent<GUITextLabel>(entity);
		lua_newtable(L);
		lua_pushnumber(L, guiTextLabel.PosX);
		lua_setfield(L, -2, "posX");
		lua_pushnumber(L, guiTextLabel.PosY);
		lua_setfield(L, -2, "posY");
		lua_pushstring(L, guiTextLabel.Text);
		lua_setfield(L, -2, "text");
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Scene::lua_SetComponent(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);

	if (type == "character")
	{
		float gold = lua_tonumber(L, 3);
		float damage = lua_tonumber(L, 4);
		LUATransform transform = lua_totransform(L, 5);
		scene->SetComponent<Character>(entity, gold, damage, transform);
		return 1;
	}
	else if (type == "backpack")
	{
		float maxWeight = lua_tonumber(L, 3);
		float currentWeight = lua_tonumber(L, 4);
		int dirtCount = lua_tointeger(L, 5);
		int stoneCount = lua_tointeger(L, 6);
		int bronzeCount = lua_tointeger(L, 7);
		int silverCount = lua_tointeger(L, 8);
		int goldCount = lua_tointeger(L, 9);
		int diamondCount = lua_tointeger(L, 10);
		int ancientRelicCount = lua_tointeger(L, 11);
		scene->SetComponent<Backpack>(entity, maxWeight, currentWeight, dirtCount, stoneCount, bronzeCount, silverCount, goldCount, diamondCount, ancientRelicCount);
		return 1;
	}
	else if (type == "mole")
	{
		float health = lua_tonumber(L, 3);
		float maxHealth = lua_tonumber(L, 4);
		float damage = lua_tonumber(L, 5);
		LUATransform transform = lua_totransform(L, 6);
		bool isStunned = lua_toboolean(L, 7);
		scene->SetComponent<Mole>(entity, health, maxHealth, damage, transform, isStunned);
		return 1;
	}
	else if (type == "shop")
	{
		int id = lua_tointeger(L, 3);
		const char* name = lua_tostring(L, 4);
		float screenPointX = lua_tonumber(L, 5);
		float screenPointY = lua_tonumber(L, 6);
		float screenWidth = lua_tonumber(L, 7);
		float screenHeight = lua_tonumber(L, 8);
		bool shopOpen = lua_toboolean(L, 9);
		LUATransform transform = lua_totransform(L, 10);
		LUAVector3 color = lua_tovector(L, 11);
		lua_pushvalue(L, 12);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		scene->SetComponent<Shop>(entity, id, name, screenPointX, screenPointY,
			screenWidth, screenHeight, shopOpen, transform, color, ref);
		return 1;
	}
	else if (type == "shopUpdate")
	{
		Shop& shop = scene->GetComponent<Shop>(entity);
		shop.ShopOpen = lua_toboolean(L, 3);
		scene->SetComponent<Shop>(shop.Id, entity, shop.Name, shop.ScreenPointX, shop.ScreenPointY,
			shop.ScreenWidth, shop.ScreenHeight, shop.ShopOpen, shop.Transform, shop.Color, shop.OnOpenRef);
		return 1;
	}
	else if (type == "button")
	{
		const char* name = lua_tostring(L, 3);
		int shopId = lua_tonumber(L, 4);
		lua_pushvalue(L, 5);
		int toolTip = luaL_ref(L, LUA_REGISTRYINDEX);
		float screenPointX = lua_tonumber(L, 6);
		float screenPointY = lua_tonumber(L, 7);
		float screenWidth = lua_tonumber(L, 8);
		float screenHeight = lua_tonumber(L, 9);
		lua_pushvalue(L, 10);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		scene->SetComponent<Button>(entity, name, shopId, toolTip, screenPointX, screenPointY, screenWidth, screenHeight, ref);
		return 1;
	}
	else if (type == "textLabel")
	{
		const char* name = lua_tostring(L, 3);
		int id = lua_tonumber(L, 4);
		int shopId = lua_tonumber(L, 5);
		float screenPointX = lua_tonumber(L, 6);
		float screenPointY = lua_tonumber(L, 7);
		float screenWidth = lua_tonumber(L, 8);
		float screenHeight = lua_tonumber(L, 9);
		scene->SetComponent<TextLabel>(entity, name, id, shopId, screenPointX, screenPointY, screenWidth, screenHeight);
		return 1;
	}
	else if (type == "gravitation")
	{
		float velocityY = lua_tonumber(L, 3);
		float gravityFactor = lua_tonumber(L, 4);
		bool isTouchingGround = lua_toboolean(L, 5);
		scene->SetComponent<Gravitation>(entity, velocityY, gravityFactor, isTouchingGround);
		return 1;
	}
	else if (type == "block")
	{
		int gold = lua_tointeger(L, 3);
		int weight = lua_tointeger(L, 4);
		LUAVector3 position = lua_tovector(L, 5);
		LUAVector3 size = lua_tovector(L, 6);
		Materials colorId = (Materials)lua_tonumber(L, 7);
		scene->SetComponent<Block>(entity, gold, weight, position, size, colorId);
		return 1;
	}
	else if (type == "treasure")
	{
		float gold = lua_tonumber(L, 3);
		float durability = lua_tonumber(L, 4);
		float maxDurability = lua_tonumber(L, 5);
		LUATransform transform = lua_totransform(L, 6);
		scene->SetComponent<Treasure>(entity, gold, durability, maxDurability, transform);
		return 1;
	}
	else if (type == "health")
	{
		float currentHealth = lua_tonumber(L, 3);
		float maxHealth = lua_tonumber(L, 4);
		scene->SetComponent<Health>(entity, currentHealth, maxHealth);
		return 1;
	}
	else if (type == "fuel")
	{
		float currentFuel = lua_tonumber(L, 3);
		float maxfuel = lua_tonumber(L, 4);
		scene->SetComponent<Fuel>(entity, currentFuel, maxfuel);
		return 1;
	}
	else if (type == "behaviour")
	{
		if (scene->HasComponents<Behaviour>(entity))
			scene->RemoveComponent<Behaviour>(entity);

		const char* path = lua_tostring(L, 3);

		// Returns the behaviour table on top of the stack
		luaL_dofile(L, path);

		// luaL_ref pops the value of the stack , so we push
		// the table again before luaL_ref
		lua_pushvalue(L, -1);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);

		// Populate the behaviour table with the information
		// the behaviour should know about
		lua_pushinteger(L, entity);
		lua_setfield(L, -2, "ID");

		lua_pushstring(L, path);
		lua_setfield(L, -2, "path");

		// Let the behaviour construct itself . It may be good
		// practice to check if the method exist before calling it
		lua_getfield(L, -1, "OnCreate");
		lua_pushvalue(L, -2); // Push the table as argument
		lua_pcall(L, 1, 0, 0);

		scene->SetComponent<Behaviour>(entity, path, ref);
		return 1;
	}
	else if (type == "transform")
	{
		scene->SetComponent<LUATransform>(entity);
		return 1;
	}
	else if (type == "guiRec")
	{
		float posX = lua_tonumber(L, 3);
		float posY = lua_tonumber(L, 4);
		float sizeX = lua_tonumber(L, 5);
		float sizeY = lua_tonumber(L, 6);
		LUAVector3 color = lua_tovector(L, 7);
		scene->SetComponent<GUIRec>(entity, posX, posY, sizeX, sizeY, color);
		return 1;
	}
	else if (type == "guiTextLabel")
	{
		float posX = lua_tonumber(L, 3);
		float posY = lua_tonumber(L, 4);
		const char* text = lua_tostring(L, 5);
		scene->SetComponent<GUITextLabel>(entity, posX, posY, text);
		return 1;
	}
	lua_pushnil(L);
	return 0;
}

int Scene::lua_RemoveComponent(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	int entity = lua_tointeger(L, 1);
	std::string type = lua_tostring(L, 2);
	if (type == "health")
		scene->RemoveComponent<Health>(entity);
	else if (type == "gravitation")
		scene->RemoveComponent<Gravitation>(entity);
	else if (type == "transform")
		scene->RemoveComponent<LUATransform>(entity);
	else if (type == "behaviour")
		scene->RemoveComponent<Behaviour>(entity);
	else if (type == "block")
		scene->RemoveComponent<Block>(entity);
	else if (type == "mole")
		scene->RemoveComponent<Mole>(entity);
	else if (type == "treasure")
		scene->RemoveComponent<Treasure>(entity);
	return 0;
}

int Scene::lua_IsKeyDown(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	const KeyboardKey key = KeyboardKey(luaL_checkinteger(L, 1));
	luaL_checkinteger(L, 1);
	lua_pushboolean(L, IsKeyDown(key));
	return 1;
}

int Scene::lua_IsKeyPressed(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	const KeyboardKey key = KeyboardKey(luaL_checkinteger(L, 1));
	luaL_checkinteger(L, 1);
	lua_pushboolean(L, IsKeyPressed(key));
	return 1;
}

int Scene::lua_IsMouseButtonPressed(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	const MouseButton button = MouseButton(luaL_checkinteger(L, 1));
	luaL_checkinteger(L, 1);
	lua_pushboolean(L, IsMouseButtonPressed(button));
	return 1;
}

int Scene::lua_GetEntityAndComponentFromClick(lua_State* L)
{
	struct CollisionInfo
	{
		entt::entity Entity;
		std::string ComponentName;
		float Distance;
	};
	std::vector<CollisionInfo> collisionInfo;
	collisionInfo.reserve(4);

	Scene* scene = lua_GetSceneUpValue(L);

	std::vector<RayCollision> collisions;
	collisions.reserve(4);
	const Ray ray = GetMouseRay({ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }, *scene->m_Camera);
	const std::vector<Materials> m_StaticBlocks = { Materials::EMPTY, Materials::SURFACE_GRASS, Materials::UNBREAKABLE,
		Materials::FINAL_LEVEL_WALL, Materials::FINAL_LEVEL_GROUND };

	{
		for (const auto& blockEntity : *scene->m_BlockEntities)
		{
			Block& block = scene->m_registry.get<Block>(blockEntity);
			if (std::find(m_StaticBlocks.cbegin(), m_StaticBlocks.cend(), block.Material) != m_StaticBlocks.cend())
			{
				continue;
			}

			const float halfBlockSizeX = block.Size.X / 2.0f;
			const float halfBlockSizeY = block.Size.Y / 2.0f;
			const float halfBlockSizeZ = block.Size.Z / 2.0f;
			const RayCollision collision = GetRayCollisionBox(ray,
				{
					{ block.Position.X - halfBlockSizeX, block.Position.Y - halfBlockSizeY, block.Position.Z - halfBlockSizeZ },
					{ block.Position.X + halfBlockSizeX, block.Position.Y + halfBlockSizeY, block.Position.Z + halfBlockSizeZ }
				}
			);

			if (collision.hit)
			{
				collisions.push_back(collision);
				// collisions.emplace_back(collision);
			}
		}

		if (collisions.size() != 0)
		{
			auto nearest = std::min_element(std::cbegin(collisions), std::cend(collisions),
				[](const RayCollision& c1, const RayCollision& c2)
				{
					return c1.distance < c2.distance;
				}
			);

			const Vector3 nearestBlock = Vector3Divide(Vector3Subtract(nearest->point, Vector3Scale(nearest->normal, 0.1f)), { 3.0f, -3.0f, 3.0f });
			const entt::entity closestEntity = (*scene->m_BlockEntities)[size_t(nearestBlock.y) * size_t(225) + size_t(nearestBlock.z) * size_t(15) + size_t(nearestBlock.x)];
			collisionInfo.push_back({ closestEntity, "block", nearest->distance });
		}
	}

	{
		CollisionInfo closestMole =
		{
			entt::entity(0),
			"mole",
			std::numeric_limits<float>::max()
		};
		scene->m_registry.view<Mole>().each([&](const entt::entity entity, const Mole& mole)
			{
				const LUAVector3 position = mole.Transform.Position;
				const LUAVector3 size = mole.Transform.Scale;
				const float halfSizeX = size.X / 2.0f;
				const float halfSizeY = size.Y / 2.0f;
				const float halfSizeZ = size.Z / 2.0f;
				const RayCollision collision = GetRayCollisionBox(ray,
					{
						{ position.X - halfSizeX, position.Y - halfSizeY, position.Z - halfSizeZ },
						{ position.X + halfSizeX, position.Y + halfSizeY, position.Z + halfSizeZ }
					}
				);

				if (collision.hit && collision.distance < closestMole.Distance)
				{
					closestMole.Entity = entity;
					closestMole.Distance = collision.distance;
				}
			});

		if (closestMole.Distance != std::numeric_limits<float>::max())
		{
			collisionInfo.push_back(closestMole);
		}
	}

	{
		CollisionInfo closestTreasure =
		{
			entt::entity(0),
			"treasure",
			std::numeric_limits<float>::max()
		};
		scene->m_registry.view<Treasure>().each([&](const entt::entity entity, const Treasure& treasure)
			{
				const LUAVector3 position = treasure.Transform.Position;
				const LUAVector3 size = treasure.Transform.Scale;
				const float halfSizeX = size.X / 2.0f;
				const float halfSizeY = size.Y / 2.0f;
				const float halfSizeZ = size.Z / 2.0f;
				const RayCollision collision = GetRayCollisionBox(ray,
					{
						{ position.X - halfSizeX, position.Y - halfSizeY, position.Z - halfSizeZ },
						{ position.X + halfSizeX, position.Y + halfSizeY, position.Z + halfSizeZ }
					}
				);

				if (collision.hit && collision.distance < closestTreasure.Distance)
				{
					closestTreasure.Entity = entity;
					closestTreasure.Distance = collision.distance;
				}
			});

		if (closestTreasure.Distance != std::numeric_limits<float>::max())
		{
			collisionInfo.push_back(closestTreasure);
		}
	}

	if (collisionInfo.empty())
	{
		lua_pushnil(L);
		lua_pushnil(L);
		lua_pushnil(L);
		return 3;
	}

	auto closestEntity = std::min_element(std::cbegin(collisionInfo), std::cend(collisionInfo),
		[](const CollisionInfo& ci1, const CollisionInfo& ci2)
		{
			return ci1.Distance < ci2.Distance;
		}
	);

	lua_pushinteger(L, int(closestEntity->Entity));
	lua_pushstring(L, closestEntity->ComponentName.c_str());
	lua_pushnumber(L, closestEntity->Distance);
	return 3;
}

Scene::~Scene()
{
	for (auto& system : m_systems)
	{
		delete system;
		system = nullptr;
	}
	//m_systems.clear();
	//m_registry.clear();
}

std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> Scene::GetBlockEntities() const
{
	return m_BlockEntities;
}

std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>> Scene::GetStaticBlockEntities() const
{
	return m_StaticBlockEntities;
}

std::shared_ptr<Camera3D> Scene::GetCamera3D() const
{
	return m_Camera;
}

void Scene::DestroySystems()
{
	for (auto& system : m_systems)
	{
		delete system;
		system = nullptr;
	}

	m_systems.clear();
	m_registry.clear();
}

void Scene::SetBlockEntities(std::array<entt::entity, NUMBER_OF_BLOCKS> blockEntities)
{
	m_BlockEntities = std::make_shared<std::array<entt::entity, NUMBER_OF_BLOCKS>>(blockEntities);
}

void Scene::SetCamera3D(std::shared_ptr<Camera3D> camera)
{
	m_Camera = camera;
}

int Scene::lua_SaveGame(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	std::ofstream saveFile;
	saveFile.open("saves\\saveFile1.sav");
	lua_getglobal(L, "CHARACTER_ENTITY_ID");
	entt::entity characterEntityId = entt::entity(lua_tointeger(L, -1));

	lua_pop(L, 1);
	if (saveFile.is_open())
	{
		scene->m_registry.view<Character, Backpack, Fuel, Health>()
			.each([&saveFile](entt::entity characterEntity, const Character& characterComponent, const Backpack& backpackComponent, const Fuel& fuelComponent, const Health& healthComponent)
				{
					saveFile << "C " << characterComponent.Gold << " " << characterComponent.Damage << " "
						<< backpackComponent.MaxWeight << " " << backpackComponent.CurrentWeight << " "
						<< backpackComponent.DirtCount << " " << backpackComponent.StoneCount << " "
						<< backpackComponent.BronzeCount << " " << backpackComponent.SilverCount << " "
						<< backpackComponent.GoldCount << " " << backpackComponent.DiamondCount << " "
						<< backpackComponent.AncientRelicCount << " "
						<< fuelComponent.CurrentFuel << " " << fuelComponent.MaxFuel << " "
						<< healthComponent.CurrentHealth << " " << healthComponent.MaxHealth << '\n';
				});

		for (const auto& blockEntity : *scene->m_BlockEntities)
		{
			const Block block = scene->m_registry.get<Block>(blockEntity);
			saveFile << "B " << block.Position.X << " " << block.Position.Y << " " << block.Position.Z << " " << block.Material << '\n';
		}

		scene->m_registry.view<Treasure>()
			.each([&saveFile](entt::entity treasureEntity, const Treasure& treasureComponent)
				{
					saveFile << "T " << treasureComponent.Gold << " " << treasureComponent.Durability << " "
						<< treasureComponent.MaxDurability << " " << treasureComponent.Transform.Position.X << " "
						<< treasureComponent.Transform.Position.Y << " " << treasureComponent.Transform.Position.Z << "\n";
				});

		saveFile.close();
	}
	else
	{
		lua_pushstring(L, "Error: Failed to save game");
	}

	return 0;
}

int Scene::lua_LoadGame(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	static constexpr std::array materialNames = { "EMPTY", "GRASS", "DIRT", "STONE", "BRONZE", "SILVER", "GOLDEN", "DIAMOND", "ANCIENT_RELIC" };
	std::ifstream saveFile;
	saveFile.open("saves\\saveFile1.sav");
	if (saveFile.is_open())
	{
		lua_getglobal(L, "blockEntityIds");
		std::size_t blocksLoaded = 0;
		std::string line;
		while (std::getline(saveFile, line))
		{
			const char componentType = line[0];
			std::vector<std::string> v;
			std::size_t startIndex = 2;
			std::size_t endIndex = line.find(" ", startIndex + 1);
			while (endIndex != std::string::npos)
			{
				v.emplace_back(line.substr(startIndex, endIndex - startIndex));
				startIndex = endIndex + 1;
				endIndex = line.find(" ", startIndex + 1);
			}
			endIndex = line.find("\n", startIndex + 1);
			v.emplace_back(line.substr(startIndex, endIndex - startIndex));

			switch (componentType)
			{
			case 'C':
			{
				scene->m_registry.view<Character>()
					.each([&](entt::entity characterEntity, Character& characterComponent)
						{
							scene->m_registry.replace<Character>(characterEntity, std::stof(v[0]), std::stof(v[1]), characterComponent.Transform);
							scene->m_registry.emplace<Backpack>(characterEntity, std::stof(v[2]), std::stof(v[3]), std::stoi(v[4]), std::stoi(v[5]), std::stoi(v[6]), std::stoi(v[7]), std::stoi(v[8]), std::stoi(v[9]), std::stoi(v[10]));
							scene->m_registry.emplace<Fuel>(characterEntity, std::stof(v[11]), std::stof(v[12]));
							scene->m_registry.emplace<Health>(characterEntity, std::stof(v[13]), std::stof(v[14]));
						});
				break;
			}
			case 'B':
			{
				lua_getglobal(L, "BLOCK_INFO");
				int materialId = std::stoi(v[3]);
				lua_getfield(L, -1, materialNames[materialId - 4]);
				lua_getfield(L, -1, "value");
				int value = lua_tointeger(L, -1);
				lua_getfield(L, -2, "weight");
				int weight = lua_tointeger(L, -1);
				lua_getglobal(L, "BLOCK_SIZE");
				float blockSize = lua_tonumber(L, -1);
				lua_pop(L, 5);
				LUAVector3 position{};
				LUAVector3 size{};
				entt::entity newEntity = entt::entity(scene->CreateEntity());
				Materials material = Materials(materialId);

				lua_getglobal(L, "helpers_setComponent");
				lua_pushinteger(L, int(newEntity));
				lua_pushstring(L, "block");
				lua_newtable(L);
				lua_pushinteger(L, value);
				lua_rawseti(L, -2, 1);
				lua_pushinteger(L, weight);
				lua_rawseti(L, -2, 2);
				lua_pushvector(L, LUAVector3{ std::stof(v[0]), std::stof(v[1]), std::stof(v[2]) });
				lua_rawseti(L, -2, 3);
				lua_pushvector(L, LUAVector3{ blockSize, blockSize, blockSize });
				lua_rawseti(L, -2, 4);
				lua_pushinteger(L, material);
				lua_rawseti(L, -2, 5);
				lua_pcall(L, 3, 0, 0);


				(*scene->m_BlockEntities)[blocksLoaded++] = newEntity;
				lua_pushinteger(L, int(newEntity));
				lua_rawseti(L, -2, blocksLoaded);
				if (blocksLoaded == scene->m_BlockEntities->size())
					lua_pop(L, 1);
				break;
			}
			case 'T':
			{
				int treasureEntity = scene->CreateEntity();
				lua_getglobal(L, "helpers_setComponent");
				lua_pushinteger(L, treasureEntity);
				lua_pushstring(L, "treasure");
				lua_newtable(L);
				lua_pushnumber(L, std::stof(v[0]));
				lua_rawseti(L, -2, 1);
				lua_pushnumber(L, std::stof(v[1]));
				lua_rawseti(L, -2, 2);
				lua_pushnumber(L, std::stof(v[2]));
				lua_rawseti(L, -2, 3);
				lua_getglobal(L, "TREASURE_SIZE");
				const float treasureSize = lua_tonumber(L, -1);
				lua_pop(L, 1);
				lua_pushtransform(L, { LUAVector3{ std::stof(v[3]), std::stof(v[4]), std::stof(v[5])}, LUAVector3{0.0f, 0.0f, 0.0f},
										LUAVector3{treasureSize, treasureSize, treasureSize} });
				lua_rawseti(L, -2, 4);
				lua_pcall(L, 3, 0, 0);

				lua_getglobal(L, "helpers_setComponent");
				lua_pushinteger(L, treasureEntity);
				lua_pushstring(L, "behaviour");
				lua_newtable(L);
				lua_getglobal(L, "LUA_FILES_FOLDER");
				std::string path = std::string{ lua_tostring(L, -1) } + "Mole.lua";
				lua_pop(L, 1);
				lua_pushstring(L, path.c_str());
				DumpStack(L);
				lua_rawseti(L, -2, 1);
				DumpStack(L);
				lua_pcall(L, 3, 0, 0);
				DumpStack(L);
				break;
			}
			}
		}

		saveFile.close();
	}
	else
	{

	}

	return 0;
}

int Scene::lua_SaveTables(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);

	luaL_checktype(L, -1, LUA_TTABLE);

	// saveFile << "saves = {}\n\n";
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		luaL_checktype(L, -1, LUA_TTABLE); // Value must be a table

		const char* keyName = lua_tostring(L, -2);
		std::ofstream saveFile;
		saveFile.open("saves\\" + std::string{ keyName } + ".lua");
		saveFile << "saves." << keyName << " = {\n";

		std::size_t count = 0;
		lua_pushnil(L);
		while (lua_next(L, -2))
		{
			++count;
			lua_pop(L, 1);
		}
		std::size_t length = std::size_t(lua_rawlen(L, -1));

		assert(!(length > 0 && count > length)); // Mixed lua tables are not allowed
		// If cond == false then raise error
		if (length != 0)
		{
			for (std::size_t i = 1; i < length + 1; ++i)
			{
				lua_rawgeti(L, -1, i);
				int type = lua_type(L, -1);
				if (type == LUA_TNUMBER)
				{
					saveFile << "\t" << lua_tointeger(L, -1) << ",\n";
				}
				else if (type == LUA_TSTRING)
				{
					saveFile << "\t\"" << lua_tostring(L, -1) << "\",\n";
				}
				else if (type == LUA_TTABLE)
				{
					saveFile << "\t{\n";

					lua_pushnil(L);
					while (lua_next(L, -2))
					{
						const char* keyName = lua_tostring(L, -2);
						int valueType = lua_type(L, -1);
						int value = lua_tonumber(L, -1);
						saveFile << "\t\t" << keyName << " = " << std::to_string(value) << ",\n";

						lua_pop(L, 1);
					}

					saveFile << "\t},\n";
				}
				lua_pop(L, 1);
			}
		}
		else
		{
			lua_pushnil(L);
			while (lua_next(L, -2))
			{
				const char* keyName = lua_tostring(L, -2);
				int valueType = lua_type(L, -1);
				saveFile << "\t" << keyName << " = ";

				if (valueType == LUA_TNUMBER)
				{
					int value = lua_tonumber(L, -1);
					saveFile << std::to_string(value) << ",\n";

				}
				else if (valueType == LUA_TSTRING)
				{
					const char* value = lua_tostring(L, -1);
					saveFile << "\"" << value << "\",\n";
				}
				else if (valueType == LUA_TTABLE)
				{
					saveFile << "{\n";

					lua_pushnil(L);
					while (lua_next(L, -2))
					{
						const char* keyName = lua_tostring(L, -2);
						int valueType = lua_type(L, -1);
						int value = lua_tonumber(L, -1);
						if (lua_isnumber(L, -2))
							saveFile << "\t\t" << std::to_string(value) << ",\n";
						else
							saveFile << "\t\t" << keyName << " = " << std::to_string(value) << ",\n";

						lua_pop(L, 1);
					}

					saveFile << "\t},\n";
				}
				lua_pop(L, 1);
			}
		}

		saveFile << "}\n";
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(L, 1);

		saveFile.close();
	}

	return 0;
}

int Scene::lua_LoadTables(lua_State* L)
{
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		const std::string filePath{ "saves\\" + std::string{ lua_tostring(L, -1) } + ".lua" };
		if (std::filesystem::exists(filePath))
			luaL_dofile(L, filePath.c_str());

		lua_pop(L, 1);
	}

	return 0;
}

int Scene::lua_DeleteFiles(lua_State* L)
{
	lua_pushnil(L);
	while (lua_next(L, -2))
	{
		const std::string filePath{ "saves\\" + std::string{ lua_tostring(L, -1) } + ".lua" };
		std::remove(filePath.c_str());
		lua_pop(L, 1);
	}

	return 0;
}

int Scene::lua_GetEntities(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	std::string type = lua_tostring(L, -1);

	if (type == "treasure")
	{
		std::size_t index = 1;
		lua_newtable(L);
		const auto treasureView = scene->m_registry.view<Treasure>();
		treasureView.each([&](entt::entity treasureEntity)
			{
				lua_pushinteger(L, lua_Integer(treasureEntity));
				lua_rawseti(L, -2, index++);
			}
		);
	}
	else
	{
		lua_pushnil(L);
	}

	return 1;
}

int Scene::lua_GetRayCenter(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);

	const Ray ray = GetMouseRay({ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f }, *scene->m_Camera);

	lua_newtable(L);
	lua_pushvector(L, { ray.position.x, ray.position.y, ray.position.z });
	lua_setfield(L, -2, "position");
	lua_pushvector(L, { ray.direction.x, ray.direction.y, ray.direction.z });
	lua_setfield(L, -2, "direction");

	return 1;
}

int Scene::lua_GetRayCollisionBox(lua_State* L)
{
	Scene* scene = lua_GetSceneUpValue(L);
	lua_getfield(L, -2, "position");
	lua_getfield(L, -3, "direction");
	lua_getfield(L, -3, "min");
	lua_getfield(L, -4, "max");
	LUAVector3 min = lua_tovector(L, -2);
	LUAVector3 max = lua_tovector(L, -1);
	LUAVector3 rayPosition = lua_tovector(L, -4);
	LUAVector3 rayDirection = lua_tovector(L, -3);
	const RayCollision collision = GetRayCollisionBox(
		{
			{ rayPosition.X, rayPosition.Y, rayPosition.Z },
			{ rayDirection.X, rayDirection.Y, rayDirection.Z }
		},
		{
			{ min.X, min.Y, min.Z },
			{ max.X, max.Y, max.Z }
		}
		);

	lua_newtable(L);
	lua_pushboolean(L, collision.hit);
	lua_setfield(L, -2, "hit");
	lua_pushnumber(L, collision.distance);
	lua_setfield(L, -2, "distance");
	lua_pushvector(L, { collision.point.x, collision.point.y, collision.point.z });
	lua_setfield(L, -2, "point");
	lua_pushvector(L, { collision.normal.x, collision.normal.y, collision.normal.z });
	lua_setfield(L, -2, "normal");

	return 1;
}

int lua_GetMouseTargetPosition(lua_State* L)
{


	lua_pushnil(L);

	return 1;
}
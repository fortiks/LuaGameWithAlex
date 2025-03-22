#pragma once

#include "System.hpp"
#include "Components.hpp"

class Scene
{
	lua_State* m_luaState;
	entt::registry m_registry;
	std::vector<System*> m_systems;
private:
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> m_BlockEntities;
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>> m_StaticBlockEntities;
	std::shared_ptr<Camera3D> m_Camera;
public:
	Scene(lua_State* L, std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> blockEntities,
		std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>> staticBlockEntities, std::shared_ptr<Camera3D> camera);
	~Scene();
public:
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> GetBlockEntities() const;
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>> GetStaticBlockEntities() const;
	std::shared_ptr<Camera3D> GetCamera3D() const;

	void DestroySystems();
	void SetBlockEntities(std::array<entt::entity, NUMBER_OF_BLOCKS> blockEntities);
	void SetCamera3D(std::shared_ptr<Camera3D> camera);
	static bool LoadGame();
public:
	// Entities

	int GetEntityCount();

	int CreateEntity();
	bool IsEntity(int entity);
	void RemoveEntity(int entity);
public:
	// Components

	template <typename... Args>
	bool HasComponents(int entity);

	template <typename T>
	T& GetComponent(int entity);

	template <typename T>
	void SetComponent(int entity, const T&);

	template <typename T, typename... Args>
	void SetComponent(int entity, Args... args);

	template <typename T>
	void RemoveComponent(int entity);

	// Systems

	template <typename T, typename... Args>
	void CreateSystem(Args... args);

	void UpdateSystems(float delta);
public:
	static void lua_openscene(lua_State* L, Scene* scene);
private:
	static Scene* lua_GetSceneUpValue(lua_State* L);

	static int lua_IsKeyDown(lua_State* L);
	static int lua_IsKeyPressed(lua_State* L);
	static int lua_IsMouseButtonPressed(lua_State* L);
	static int lua_GetEntityAndComponentFromClick(lua_State* L);
	static int lua_SaveGame(lua_State* L);
	static int lua_SaveTables(lua_State* L);
	static int lua_DeleteFiles(lua_State* L);
	static int lua_LoadTables(lua_State* L);
	static int lua_LoadGame(lua_State* L);
	static int lua_GetRayCenter(lua_State* L);
	static int lua_GetRayCollisionBox(lua_State* L);
	static int lua_GetMouseTargetPosition(lua_State* L);

	static int lua_GetEntityCount(lua_State* L);
	static int lua_CreateEntity(lua_State* L);
	static int lua_IsEntity(lua_State* L);
	static int lua_RemoveEntity(lua_State* L);
	static int lua_GetEntities(lua_State* L);

	static int lua_HasComponent(lua_State* L);
	static int lua_GetComponent(lua_State* L);
	static int lua_SetComponent(lua_State* L);
	static int lua_RemoveComponent(lua_State* L);
};

// Template definitions after class definition
template <typename... Args>
bool Scene::HasComponents(int entity)
{
	return m_registry.all_of<Args... >(static_cast<entt::entity>(entity));
}

template <typename T>
T& Scene::GetComponent(int entity)
{
	return m_registry.get<T>(static_cast<entt::entity>(entity));
}

template <typename T>
void Scene::SetComponent(int entity, const T& component)
{
	m_registry.emplace_or_replace<T>(static_cast<entt::entity>(entity), component);
}

template <typename T, typename... Args>
void Scene::SetComponent(int entity, Args... args)
{
	m_registry.emplace_or_replace<T>(static_cast<const entt::entity>(entity), args...);
}

template <typename T>
void Scene::RemoveComponent(int entity)
{
	m_registry.remove<T>(static_cast<entt::entity>(entity));
}

template <typename T, typename... Args>
void Scene::CreateSystem(Args... args)
{
	m_systems.emplace_back(new T(args ...));
}

#include <iostream>
#include <thread>
#include <string>
#include <chrono>

#include "Scene.hpp"

extern "C" {
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
}

struct Paths
{
	static constexpr const char* Helpers()
	{
		return "src\\Helpers.lua";
	}
	static constexpr const char* NewGame()
	{
		return "src\\NewGame.lua";
	}
	static constexpr const char* LoadGame()
	{
		return "src\\LoadGame.lua";
	}
	static constexpr const char* InitShops()
	{
		return "src\\InitShops.lua";
	}
	static constexpr const char* InitScene()
	{
		return "src\\InitScene.lua";
	}
	static constexpr const char* EditorBehaviour()
	{
		return "src\\Editor.lua";
	}
};

void ResetState(Scene* scene, lua_State* L, std::shared_ptr<Camera3D>& camera,
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>>& staticBlockEntities)
{
	camera = std::make_shared<Camera3D>(Camera3D{ { 0.0f, 10.0f, 0.0f }, { -5.0f, 5.0f, -5.0f },
												{ 0.0f, 1.0f, 0.0f }, 45.0f, CAMERA_PERSPECTIVE });
	// scene->SetCamera3D(camera);
	scene->DestroySystems();

	luaL_dofile(L, Paths::InitScene());
	luaL_dofile(L, Paths::InitShops());

	lua_getglobal(L, "staticBlockEntityIds");
	for (size_t blockNr = 0; blockNr < NUMBER_OF_STATIC_BLOCKS; ++blockNr)
	{
		lua_rawgeti(L, -1, blockNr + 1);
		(*staticBlockEntities)[blockNr] = entt::entity(lua_tointeger(L, -1));
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
}

enum GAME_STATE {
	IN_MENU = 0,
	EDIT_MODE,
	PLAYING,
	PLAYER_WON,
	PLAYER_WON_MENU,
	GAME_OVER,
	GAME_OVER_MENU,
	QUIT
};

void SetUpGame() {}

int main()
{
	GAME_STATE currentState = GAME_STATE::IN_MENU;
	constexpr const int SCREEN_WIDTH = 1768;
	constexpr const int SCREEN_HEIGHT = 992;

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lua Project");
	SetTargetFPS(60);
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>> staticBlockEntities =
		std::make_shared<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>>();
	std::shared_ptr<Camera3D> camera = std::make_shared<Camera3D>(Camera3D{ { 0.0f, 10.0f, 0.0f }, { -5.0f, 5.0f, -5.0f },
																	{ 0.0f, 1.0f, 0.0f }, 45.0f, CAMERA_PERSPECTIVE });
	Scene scene{ L, nullptr, staticBlockEntities, camera };
	Scene::lua_openscene(L, &scene);
	luaL_dofile(L, Paths::InitScene());
	luaL_dofile(L, Paths::InitShops());
	luaL_dofile(L, Paths::Helpers());

	lua_getglobal(L, "staticBlockEntityIds");
	for (size_t blockNr = 0; blockNr < NUMBER_OF_STATIC_BLOCKS; ++blockNr)
	{
		lua_rawgeti(L, -1, blockNr + 1);
		(*staticBlockEntities)[blockNr] = entt::entity(lua_tointeger(L, -1));
		lua_pop(L, 1);
	}
	lua_pop(L, 1);

	using Clock = std::chrono::high_resolution_clock;
	Clock::time_point lastTick = Clock::now();

	lua_getglobal(L, "START_Y");
	float startY = lua_tonumber(L, -1);
	lua_pop(L, 1);

	Clock::time_point gameTime;
	while (!WindowShouldClose())
	{
		lastTick = Clock::now();
		switch (currentState)
		{
		case GAME_STATE::IN_MENU:
		{
			EnableCursor();
			BeginDrawing();
			{
				GuiSetStyle(DEFAULT, TEXT_SIZE, 80);
				GuiLabel({ SCREEN_WIDTH / 3.0f - (SCREEN_WIDTH / 4.0f) / 2.0f, SCREEN_HEIGHT * 0.1f - SCREEN_HEIGHT * 0.5f / 2.0f,
							SCREEN_WIDTH / 4.0f, SCREEN_HEIGHT * 0.5f }, "Hero of the Underground");

				GuiSetStyle(DEFAULT, TEXT_SIZE, 50);
				GuiLabel({ SCREEN_WIDTH * 0.8f - (SCREEN_WIDTH / 3.0f) / 2.0f, SCREEN_HEIGHT * 0.4f,
							SCREEN_WIDTH / 3.0f, SCREEN_HEIGHT * 0.2f }, "Credits");
				GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
				GuiLabel({ SCREEN_WIDTH * 0.8f - (SCREEN_WIDTH / 3.0f) / 2.0f, SCREEN_HEIGHT * 0.5f,
							SCREEN_WIDTH / 3.0f, SCREEN_HEIGHT * 0.2f }, "Alex Olsson");
				GuiLabel({ SCREEN_WIDTH * 0.8f - (SCREEN_WIDTH / 3.0f) / 2.0f, SCREEN_HEIGHT * 0.6f,
							SCREEN_WIDTH / 3.0f, SCREEN_HEIGHT * 0.2f }, "Axel Oseen");

				GuiSetStyle(DEFAULT, TEXT_SIZE, 50);
				if (GuiButton({ SCREEN_WIDTH * 1.0f / 8.0f, SCREEN_HEIGHT / 3.0f, SCREEN_WIDTH * 0.25f, SCREEN_HEIGHT * 0.1f }, "New Game"))
				{
					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);
					luaL_dofile(L, Paths::NewGame());

					currentState = GAME_STATE::PLAYING;
				}
				else if (GuiButton({ SCREEN_WIDTH * 1.0f / 8.0f, SCREEN_HEIGHT / 3.0f + SCREEN_HEIGHT * 0.125f, SCREEN_WIDTH * 0.25f, SCREEN_HEIGHT * 0.1f }, "Load Game"))
				{
					GuiSetStyle(DEFAULT, TEXT_SIZE, 20);

					luaL_dofile(L, Paths::LoadGame());
					bool couldLoad = lua_toboolean(L, -1);
					if (!couldLoad)
						break;

					currentState = GAME_STATE::PLAYING;
				}
				else if (GuiButton({ SCREEN_WIDTH * 1.0f / 8.0f, SCREEN_HEIGHT / 3.0f + SCREEN_HEIGHT * 0.25f, SCREEN_WIDTH * 0.25f, SCREEN_HEIGHT * 0.1f }, "Editor mode"))
				{
					currentState = GAME_STATE::EDIT_MODE;
				}
				else if (GuiButton({ SCREEN_WIDTH * 1.0f / 8.0f, SCREEN_HEIGHT / 3.0f + SCREEN_HEIGHT * 0.375f, SCREEN_WIDTH * 0.25f, SCREEN_HEIGHT * 0.1f }, "Exit"))
				{
					currentState = GAME_STATE::QUIT;
				}

				ClearBackground(DARKBLUE);
			}
			EndDrawing();
			break;
		}
		case GAME_STATE::EDIT_MODE:
		{
			scene.CreateSystem<FreeCameraSystem>(scene.GetCamera3D());
			scene.CreateSystem<BehaviourSystem>(L);
			luaL_dostring(L, R"(
				scene.SetComponent(scene.CreateEntity(), "behaviour", LUA_FILES_FOLDER .. "Editor.lua")
			)");
			//scene.SetComponent<Behaviour>(scene.CreateEntity(), Paths::EditorBehaviour());

			lua_getglobal(L, "blockEntityIds");
			std::array<entt::entity, NUMBER_OF_BLOCKS> blockEntities{};
			for (size_t blockNr = 0; blockNr < NUMBER_OF_BLOCKS; ++blockNr)
			{
				lua_rawgeti(L, -1, blockNr + 1);
				blockEntities[blockNr] = entt::entity(lua_tointeger(L, -1));
				lua_pop(L, 1);
			}
			lua_pop(L, 1);

			scene.SetBlockEntities(blockEntities);
			scene.CreateSystem<EditorRendererSystem>(L, SCREEN_WIDTH, SCREEN_HEIGHT, scene.GetBlockEntities(), camera);

			while (!WindowShouldClose())
			{
				Clock::time_point thisTick = Clock::now();
				Clock::duration delta = thisTick - lastTick;
				lastTick = thisTick;

				lua_getglobal(L, "IS_IN_EDIT_MODE");
				if (!lua_toboolean(L, -1))
				{
					ResetState(&scene, L, scene.GetCamera3D(), staticBlockEntities);
					luaL_dofile(L, Paths::LoadGame());
					currentState = GAME_STATE::PLAYING;

					break;
				}

				float deltaInSeconds = delta.count() / 1000000000.0f;
				scene.UpdateSystems(deltaInSeconds);
			}
			
			if (currentState != GAME_STATE::PLAYING)
				currentState = GAME_STATE::QUIT;
			
			break;
		}
		case GAME_STATE::PLAYING:
		{
			bool playerHasWon = false;
			bool playerIsAlive = true;
			bool isEditing = false;

			lua_getglobal(L, "blockEntityIds");
			std::array<entt::entity, NUMBER_OF_BLOCKS> blockEntities{};
			for (size_t blockNr = 0; blockNr < NUMBER_OF_BLOCKS; ++blockNr)
			{
				lua_rawgeti(L, -1, blockNr + 1);
				blockEntities[blockNr] = entt::entity(lua_tointeger(L, -1));
				lua_pop(L, 1);
			}
			lua_pop(L, 1);

			scene.SetBlockEntities(blockEntities);

			scene.CreateSystem<CameraSystem>(L, scene.GetCamera3D());
			scene.CreateSystem<CollisionSystem>(L, scene.GetBlockEntities(), scene.GetStaticBlockEntities());
			scene.CreateSystem<RendererSystem>(L, SCREEN_WIDTH, SCREEN_HEIGHT, scene.GetBlockEntities(), scene.GetStaticBlockEntities(), scene.GetCamera3D());
			scene.CreateSystem<BehaviourSystem>(L);
			scene.CreateSystem<GravitationSystem>(L, scene.GetBlockEntities(), scene.GetStaticBlockEntities(), startY);


			while (!playerHasWon && playerIsAlive && !isEditing)
			{
				if (WindowShouldClose())
				{
					currentState = GAME_STATE::QUIT;
					break;
				}

				Clock::time_point thisTick = Clock::now();
				Clock::duration delta = thisTick - lastTick;
				lastTick = thisTick;

				lua_getglobal(L, "PLAYER_WON");
				if (lua_toboolean(L, -1))
				{
					playerHasWon = true;
				}
				lua_getglobal(L, "PLAYER_ALIVE");
				if (!lua_toboolean(L, -1))
				{
					playerIsAlive = false;
				}
				lua_getglobal(L, "IS_IN_EDIT_MODE");
				if (lua_toboolean(L, -1))
				{
					isEditing = true;
				}
				lua_pop(L, 2);

				float deltaInSeconds = delta.count() / 1000000000.0f;
				scene.UpdateSystems(deltaInSeconds);
			}

			if (playerHasWon)
			{
				currentState = GAME_STATE::PLAYER_WON;
			}
			else if (!playerIsAlive)
			{
				currentState = GAME_STATE::GAME_OVER;
			}
			else if (isEditing)
			{
				ResetState(&scene, L, scene.GetCamera3D(), staticBlockEntities);
				currentState = GAME_STATE::EDIT_MODE;
			}

			break;
		}
		case GAME_STATE::PLAYER_WON:
		{
			ResetState(&scene, L, camera, staticBlockEntities);

			currentState = GAME_STATE::PLAYER_WON_MENU;
			break;
		}
		case GAME_STATE::PLAYER_WON_MENU:
		{
			GuiSetStyle(DEFAULT, TEXT_SIZE, 80);
			EnableCursor();
			BeginDrawing();
			{
				ClearBackground(GREEN);
				GuiLabel({ SCREEN_WIDTH * 0.37f, SCREEN_HEIGHT * 0.2f, SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.2f }, "You won!");
				if (GuiButton({ SCREEN_WIDTH * 0.25f, SCREEN_HEIGHT * 0.5f, SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.3f }, "Return to menu"))
				{
					currentState = GAME_STATE::IN_MENU;
				}
			}
			EndDrawing();

			break;
		}
		case GAME_STATE::GAME_OVER:
		{
			ResetState(&scene, L, camera, staticBlockEntities);
			currentState = GAME_STATE::GAME_OVER_MENU;

			break;
		}
		case GAME_STATE::GAME_OVER_MENU:
		{
			GuiSetStyle(DEFAULT, TEXT_SIZE, 80);
			EnableCursor();
			BeginDrawing();
			{
				ClearBackground(RED);
				GuiLabel({ SCREEN_WIDTH * 0.37f, SCREEN_HEIGHT * 0.2f, SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.2f }, "You lost!");
				if (GuiButton({ SCREEN_WIDTH * 0.25f, SCREEN_HEIGHT * 0.5f, SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.3f }, "Return to menu"))
				{
					currentState = GAME_STATE::IN_MENU;
				}
			}
			EndDrawing();
			break;
		}
		case GAME_STATE::QUIT:
		{
			luaL_dostring(L, R"(
				scene.DeleteFiles({
					"editorBlocks",
					"editorTreasures",
					"tempCharacter"
				})
			)");

			lua_close(L);
			return 0;
		}
		}
	}

	lua_close(L);
	return 0;
}
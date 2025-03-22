#pragma once

#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <math.h>
#include <limits>
#include <memory>
#include <string>

#include "Components.hpp"
#include "raylib.h"
#include "raymath.h"
#include "entt.h"

extern "C" {
#include "raylib.h"

#define RAYGUI_STATIC
#include "raygui.h"
}

static constexpr float DEG_TO_RAD = 0.0174532925f;
static constexpr size_t NUMBER_OF_BLOCKS = size_t(6975);
static constexpr size_t NUMBER_OF_STATIC_BLOCKS = size_t(13);

class System
{
public:
	virtual bool OnUpdate(entt::registry& registry, float delta) = 0;
};

class BehaviourSystem : public System
{
	lua_State* m_L;
public:
	BehaviourSystem(lua_State* L) : m_L(L) {}
	bool OnUpdate(entt::registry& registry, float delta) final
	{
		const auto behaviourView = registry.view<Behaviour>();

		behaviourView.each([&](Behaviour& script) {
			// Retrieve the behaviour table to the top of the stack
			lua_rawgeti(m_L, LUA_REGISTRYINDEX, script.LuaRef);

			// Retrieve the OnUpdate method from the table
			lua_getfield(m_L, -1, "OnUpdate");

			// Push the table as the first argument to the method
			lua_pushvalue(m_L, -2);

			// Push delta as the second argument to the method
			lua_pushnumber(m_L, delta);

			// Call the method, pops the method and its arguments
			// from the stack
			if (lua_pcall(m_L, 2, 0, 0) != LUA_OK)
				DumpError(m_L);

			// Pop the behaviour table from the stack
			lua_pop(m_L, 1);
			});

		return false;
	}
};

class FreeCameraSystem : public System
{
private:
	// lua_State* m_L;
	std::shared_ptr<Camera3D> m_Camera;
private:
	typedef enum {
		MOVE_FRONT = 0,
		MOVE_BACK,
		MOVE_RIGHT,
		MOVE_LEFT,
		MOVE_UP,
		MOVE_DOWN
	} CameraMove;
	typedef struct {
		unsigned int mode;              // Current camera mode
		float targetDistance;           // Camera distance from position to target
		float playerEyesPosition;       // Player eyes position from ground (in meters)
		Vector2 angle;                  // Camera angle in plane XZ
		Vector2 previousMousePosition;  // Previous mouse position

		// Camera movement control keys
		int moveControl[6];             // Move controls (CAMERA_FIRST_PERSON)
		int smoothZoomControl;          // Smooth zoom control key
		int altControl;                 // Alternative control key
		int panControl;                 // Pan view control key
	} CameraData;
	CameraData m_CameraState;
	static constexpr float PLAYER_MOVEMENT_SENSITIVITY = 5.0f;
	static constexpr float CAMERA_FREE_PANNING_DIVIDER = 5.1f;
	static constexpr float CAMERA_FIRST_PERSON_MIN_CLAMP = 89.0f;
	static constexpr float CAMERA_FIRST_PERSON_MAX_CLAMP = -89.0f;
	static constexpr float CAMERA_MOUSE_MOVE_SENSITIVITY = 0.003f;
public:
	FreeCameraSystem(std::shared_ptr<Camera3D> camera)
		: m_Camera(camera)
	{
		m_CameraState = {
			0u,
			0.0f,
			1.85f,
			{0.0f, 0.0f },
			{ 0.0f, 0.0f },
			{ 'W', 'S', 'D', 'A', ' ', 'Q' },
			341,		// raylib: KEY_LEFT_CONTROL
			342,		// raylib: KEY_LEFT_ALT
			2			// raylib: MOUSE_BUTTON_MIDDLE
		};

		const Vector3 v1 = camera->position;
		const Vector3 v2 = camera->target;
		const float dx = v2.x - v1.x;
		const float dy = v2.y - v1.y;
		const float dz = v2.z - v1.z;
		m_CameraState.targetDistance = sqrtf(dx * dx + dy * dy + dz * dz);   // Distance to target
		m_CameraState.angle.x = atan2f(dx, dz);                        // Camera angle in plane XZ (0 aligned with Z, move positive CCW)
		m_CameraState.angle.y = atan2f(dy, sqrtf(dx * dx + dz * dz));      // Camera angle in plane XY (0 aligned with X, move positive CW)
		m_CameraState.playerEyesPosition = camera->position.y;          // Init player eyes position to camera Y position
		m_CameraState.previousMousePosition = GetMousePosition();      // Init mouse position
		m_CameraState.mode = CAMERA_FIRST_PERSON;
		DisableCursor();
	}

	bool OnUpdate(entt::registry& registry, float delta) final
	{
		// Mouse movement detection
		Vector2 mousePositionDelta = { 0.0f, 0.0f };
		const Vector2 mousePosition = GetMousePosition();
		// float mouseWheelMove = GetMouseWheelMove();

		// Keys input detection
		const bool direction[6] = { IsKeyDown(m_CameraState.moveControl[MOVE_FRONT]),
							  IsKeyDown(m_CameraState.moveControl[MOVE_BACK]),
							  IsKeyDown(m_CameraState.moveControl[MOVE_RIGHT]),
							  IsKeyDown(m_CameraState.moveControl[MOVE_LEFT]),
							  IsKeyDown(m_CameraState.moveControl[MOVE_UP]),
							  IsKeyDown(m_CameraState.moveControl[MOVE_DOWN]) };

		mousePositionDelta.x = mousePosition.x - m_CameraState.previousMousePosition.x;
		mousePositionDelta.y = mousePosition.y - m_CameraState.previousMousePosition.y;

		m_CameraState.previousMousePosition = mousePosition;

		m_Camera->position.x += 5 * ((sinf(m_CameraState.angle.x) * direction[MOVE_BACK] -
			sinf(m_CameraState.angle.x) * direction[MOVE_FRONT] -
			cosf(m_CameraState.angle.x) * direction[MOVE_LEFT] +
			cosf(m_CameraState.angle.x) * direction[MOVE_RIGHT]) / PLAYER_MOVEMENT_SENSITIVITY);

		m_Camera->position.y += IsKeyDown(KEY_SPACE) ? 5 * 0.30f : 0.0f;
		m_Camera->position.y += IsKeyDown(KEY_LEFT_SHIFT) ? 5 * -0.30f : 0.0f;

		m_Camera->position.z += 5 * ((cosf(m_CameraState.angle.x) * direction[MOVE_BACK] -
			cosf(m_CameraState.angle.x) * direction[MOVE_FRONT] +
			sinf(m_CameraState.angle.x) * direction[MOVE_LEFT] -
			sinf(m_CameraState.angle.x) * direction[MOVE_RIGHT]) / PLAYER_MOVEMENT_SENSITIVITY);

		// Camera orientation calculation
		m_CameraState.angle.x += (mousePositionDelta.x * -CAMERA_MOUSE_MOVE_SENSITIVITY);
		m_CameraState.angle.y += (mousePositionDelta.y * -CAMERA_MOUSE_MOVE_SENSITIVITY);

		// Angle clamp
		if (m_CameraState.angle.y > CAMERA_FIRST_PERSON_MIN_CLAMP * DEG2RAD) m_CameraState.angle.y = CAMERA_FIRST_PERSON_MIN_CLAMP * DEG2RAD;
		else if (m_CameraState.angle.y < CAMERA_FIRST_PERSON_MAX_CLAMP * DEG2RAD) m_CameraState.angle.y = CAMERA_FIRST_PERSON_MAX_CLAMP * DEG2RAD;

		const Matrix matTranslation = { 1.0f, 0.0f, 0.0f, 0.0f,
								  0.0f, 1.0f, 0.0f, 0.0f,
								  0.0f, 0.0f, 1.0f, (m_CameraState.targetDistance / CAMERA_FREE_PANNING_DIVIDER),
								  0.0f, 0.0f, 0.0f, 1.0f };

		const float cosz = cosf(0.0f);
		const float sinz = sinf(0.0f);
		const float cosy = cosf(-(PI * 2.0f - m_CameraState.angle.x));
		const float siny = sinf(-(PI * 2.0f - m_CameraState.angle.x));
		const float cosx = cosf(-(PI * 2.0f - m_CameraState.angle.y));
		const float sinx = sinf(-(PI * 2.0f - m_CameraState.angle.y));

		const Matrix matRotation = { cosz * cosy, (cosz * siny * sinx) - (sinz * cosx), (cosz * siny * cosx) + (sinz * sinx), 0.0f,
							   sinz * cosy, (sinz * siny * sinx) + (cosz * cosx), (sinz * siny * cosx) - (cosz * sinx), 0.0f,
							   -siny, cosy * sinx, cosy * cosx, 0.0f,
							   0.0f, 0.0f, 0.0f, 1.0f };

		const float tx = matTranslation.m12 * matRotation.m0 + matTranslation.m13 * matRotation.m4 + matTranslation.m14 * matRotation.m8 + matTranslation.m15 * matRotation.m12;
		const float ty = matTranslation.m12 * matRotation.m1 + matTranslation.m13 * matRotation.m5 + matTranslation.m14 * matRotation.m9 + matTranslation.m15 * matRotation.m13;
		const float tz = matTranslation.m12 * matRotation.m2 + matTranslation.m13 * matRotation.m6 + matTranslation.m14 * matRotation.m10 + matTranslation.m15 * matRotation.m14;
		m_Camera->target.x = m_Camera->position.x - tx;
		m_Camera->target.y = m_Camera->position.y - ty;
		m_Camera->target.z = m_Camera->position.z - tz;

		return false;
	}
};

class EditorRendererSystem : public System
{
	lua_State* m_L;
	int m_ScreenWidth;
	int m_ScreenHeight;
	std::shared_ptr<Camera3D> m_Camera;
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> m_BlockEntities;
	std::vector<BoundingBox> m_BoundingBoxes;
public:
	EditorRendererSystem(lua_State* L, int screenWidth, int screenHeight, std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> blockEntities, std::shared_ptr<Camera3D>& camera)
		: m_L(L), m_ScreenWidth(screenWidth), m_ScreenHeight(screenHeight), m_BlockEntities(blockEntities), m_Camera(camera)
	{
	}

	bool OnUpdate(entt::registry& registry, float delta) final
	{
		BeginDrawing();
		{
			ClearBackground(SKYBLUE);
			BeginMode3D(*m_Camera);
			{
				registry.view<Treasure>().each([](const Treasure& treasure)
					{
						DrawCube({ treasure.Transform.Position.X, treasure.Transform.Position.Y, treasure.Transform.Position.Z },
							treasure.Transform.Scale.X, treasure.Transform.Scale.Y, treasure.Transform.Scale.Z, BLACK);
					}
				);

				lua_getglobal(m_L, "TARGET_BOUNDING_BOX");
				if (lua_isnil(m_L, -1))
				{
					lua_pop(m_L, 1);
				}
				else
				{
					lua_getfield(m_L, -1, "center");
					lua_getfield(m_L, -2, "size");
					const LUAVector3 center = lua_tovector(m_L, -2);
					const LUAVector3 size = lua_tovector(m_L, -1);
					lua_pop(m_L, 3);

					DrawCubeWiresV(
						{ center.X, center.Y, center.Z },
						{ size.X, size.Y, size.Z },
						{ 51, 223, 255, 255 }
					);
				}

				for (const auto& blockEntity : *m_BlockEntities)
				{
					const Block& blockToDraw = registry.get<Block>(blockEntity);
					if (blockToDraw.Material == Materials::EMPTY)
						continue;

					const Color blockColor = [&]() -> Color
					{
						switch (blockToDraw.Material)
						{
						case Materials::GRASS:
						{
							return { 86, 125, 70, 255 };
						}
						case Materials::DIRT:
						{
							return DARKBROWN;
						}
						case Materials::STONE:
						{
							return GRAY;
						}
						case Materials::BRONZE:
						{
							return { 205, 127, 50, 255 };
						}
						case Materials::SILVER:
						{
							return { 192, 192, 192, 255 };
						}
						case Materials::GOLDEN:
						{
							return GOLD;
						}
						case Materials::DIAMOND:
						{
							return SKYBLUE;
						}
						case Materials::ANCIENT_RELIC:
						{
							return BEIGE;
						}
						default:
							return DARKBROWN;
						}
					}();

					DrawCube({ blockToDraw.Position.X, blockToDraw.Position.Y, blockToDraw.Position.Z },
						blockToDraw.Size.X, blockToDraw.Size.Y, blockToDraw.Size.Z, blockColor);
				}
			}
			EndMode3D();

			registry.view<GUIRec>().each([this](GUIRec& guiRec)
				{
					DrawRectangleV({ guiRec.PosX * m_ScreenWidth, guiRec.PosY * m_ScreenHeight },
						{ guiRec.SizeX * m_ScreenWidth, guiRec.SizeY * m_ScreenHeight },
						{ unsigned char(guiRec.Color.X), unsigned char(guiRec.Color.Y), unsigned char(guiRec.Color.Z), unsigned char(255) });
				}
			);
			registry.view<GUITextLabel>().each([this](GUITextLabel& guiTextLabel)
				{
					DrawText(guiTextLabel.Text, guiTextLabel.PosX * m_ScreenWidth, guiTextLabel.PosY * m_ScreenHeight, 24, BLACK);
				}
			);

			DrawCircleV({ m_ScreenWidth / 2.0f, m_ScreenHeight / 2.0f }, 3.0f, WHITE);
			DrawFPS(m_ScreenWidth - m_ScreenWidth * 0.05, m_ScreenHeight * 0.025);
		}

		EndDrawing();
		return false;
	}
};

class CameraSystem : public System
{
private:
	lua_State* m_L;
	std::shared_ptr<Camera3D> m_Camera;
private:
	typedef enum {
		MOVE_FRONT = 0,
		MOVE_BACK,
		MOVE_RIGHT,
		MOVE_LEFT,
		MOVE_UP,
		MOVE_DOWN
	} CameraMove;
	typedef struct {
		unsigned int mode;              // Current camera mode
		float targetDistance;           // Camera distance from position to target
		float playerEyesPosition;       // Player eyes position from ground (in meters)
		Vector2 angle;                  // Camera angle in plane XZ
		Vector2 previousMousePosition;  // Previous mouse position

		// Camera movement control keys
		int moveControl[6];             // Move controls (CAMERA_FIRST_PERSON)
		int smoothZoomControl;          // Smooth zoom control key
		int altControl;                 // Alternative control key
		int panControl;                 // Pan view control key
	} CameraData;
	CameraData m_CameraState;
	static constexpr float PLAYER_MOVEMENT_SENSITIVITY = 5.0f;
	static constexpr float CAMERA_FREE_PANNING_DIVIDER = 5.1f;
	static constexpr float CAMERA_FIRST_PERSON_MIN_CLAMP = 89.0f;
	static constexpr float CAMERA_FIRST_PERSON_MAX_CLAMP = -89.0f;
	static constexpr float CAMERA_MOUSE_MOVE_SENSITIVITY = 0.003f;
public:
	CameraSystem(lua_State* L, std::shared_ptr<Camera3D> camera)
		: m_L(L), m_Camera(camera)
	{
		m_CameraState = {
			0u,
			0.0f,
			1.85f,
			{0.0f, 0.0f },
			{ 0.0f, 0.0f },
			{ 'W', 'S', 'D', 'A', ' ', 'Q' },
			341,		// raylib: KEY_LEFT_CONTROL
			342,		// raylib: KEY_LEFT_ALT
			2			// raylib: MOUSE_BUTTON_MIDDLE
		};

		const Vector3 v1 = camera->position;
		const Vector3 v2 = camera->target;
		const float dx = v2.x - v1.x;
		const float dy = v2.y - v1.y;
		const float dz = v2.z - v1.z;
		m_CameraState.targetDistance = sqrtf(dx * dx + dy * dy + dz * dz);   // Distance to target
		m_CameraState.angle.x = atan2f(dx, dz);                        // Camera angle in plane XZ (0 aligned with Z, move positive CCW)
		m_CameraState.angle.y = atan2f(dy, sqrtf(dx * dx + dz * dz));      // Camera angle in plane XY (0 aligned with X, move positive CW)
		m_CameraState.playerEyesPosition = camera->position.y;          // Init player eyes position to camera Y position
		m_CameraState.previousMousePosition = GetMousePosition();      // Init mouse position
		m_CameraState.mode = CAMERA_FIRST_PERSON;
		DisableCursor();
	}
	bool OnUpdate(entt::registry& registry, float delta) final
	{
		// Mouse movement detection
		Vector2 mousePositionDelta = { 0.0f, 0.0f };
		const Vector2 mousePosition = GetMousePosition();
		// float mouseWheelMove = GetMouseWheelMove();

		// Keys input detection
		const bool direction[6] = { IsKeyDown(m_CameraState.moveControl[MOVE_FRONT]),
							  IsKeyDown(m_CameraState.moveControl[MOVE_BACK]),
							  IsKeyDown(m_CameraState.moveControl[MOVE_RIGHT]),
							  IsKeyDown(m_CameraState.moveControl[MOVE_LEFT]),
							  IsKeyDown(m_CameraState.moveControl[MOVE_UP]),
							  IsKeyDown(m_CameraState.moveControl[MOVE_DOWN]) };

		mousePositionDelta.x = mousePosition.x - m_CameraState.previousMousePosition.x;
		mousePositionDelta.y = mousePosition.y - m_CameraState.previousMousePosition.y;
		m_CameraState.previousMousePosition = mousePosition;

		// Camera orientation calculation
		m_CameraState.angle.x += (mousePositionDelta.x * -CAMERA_MOUSE_MOVE_SENSITIVITY);
		m_CameraState.angle.y += (mousePositionDelta.y * -CAMERA_MOUSE_MOVE_SENSITIVITY);

		// Angle clamp
		if (m_CameraState.angle.y > CAMERA_FIRST_PERSON_MIN_CLAMP * DEG2RAD) m_CameraState.angle.y = CAMERA_FIRST_PERSON_MIN_CLAMP * DEG2RAD;
		else if (m_CameraState.angle.y < CAMERA_FIRST_PERSON_MAX_CLAMP * DEG2RAD) m_CameraState.angle.y = CAMERA_FIRST_PERSON_MAX_CLAMP * DEG2RAD;

		const Matrix matTranslation = { 1.0f, 0.0f, 0.0f, 0.0f,
								  0.0f, 1.0f, 0.0f, 0.0f,
								  0.0f, 0.0f, 1.0f, (m_CameraState.targetDistance / CAMERA_FREE_PANNING_DIVIDER),
								  0.0f, 0.0f, 0.0f, 1.0f };

		const float cosz = cosf(0.0f);
		const float sinz = sinf(0.0f);
		const float cosy = cosf(-(PI * 2.0f - m_CameraState.angle.x));
		const float siny = sinf(-(PI * 2.0f - m_CameraState.angle.x));
		const float cosx = cosf(-(PI * 2.0f - m_CameraState.angle.y));
		const float sinx = sinf(-(PI * 2.0f - m_CameraState.angle.y));

		const Matrix matRotation = { cosz * cosy, (cosz * siny * sinx) - (sinz * cosx), (cosz * siny * cosx) + (sinz * sinx), 0.0f,
							   sinz * cosy, (sinz * siny * sinx) + (cosz * cosx), (sinz * siny * cosx) - (cosz * sinx), 0.0f,
							   -siny, cosy * sinx, cosy * cosx, 0.0f,
							   0.0f, 0.0f, 0.0f, 1.0f };

		const float tx = matTranslation.m12 * matRotation.m0 + matTranslation.m13 * matRotation.m4 + matTranslation.m14 * matRotation.m8 + matTranslation.m15 * matRotation.m12;
		const float ty = matTranslation.m12 * matRotation.m1 + matTranslation.m13 * matRotation.m5 + matTranslation.m14 * matRotation.m9 + matTranslation.m15 * matRotation.m13;
		const float tz = matTranslation.m12 * matRotation.m2 + matTranslation.m13 * matRotation.m6 + matTranslation.m14 * matRotation.m10 + matTranslation.m15 * matRotation.m14;


		const auto characterView = registry.view<Character>();
		characterView.each([&](entt::entity entity, Character& characterComponent)
			{
				const Vector2 toTargetXZNorm = Vector2Normalize({ (m_Camera->target.x - m_Camera->position.x), (m_Camera->target.z - m_Camera->position.z) });
				characterComponent.Transform.Rotation = { toTargetXZNorm.x, 0.0f, toTargetXZNorm.y };

				m_Camera->position.x = characterComponent.Transform.Position.X;
				m_Camera->position.y = characterComponent.Transform.Position.Y + characterComponent.Transform.Scale.Y / 2.0f / 3.0f;
				m_Camera->position.z = characterComponent.Transform.Position.Z;
				m_Camera->target.x = m_Camera->position.x - tx;
				m_Camera->target.y = m_Camera->position.y - ty;
				m_Camera->target.z = m_Camera->position.z - tz;
			}
		);

		return false;
	}
};

class GravitationSystem : public System
{
	lua_State* m_L;
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> m_BlockEntities;
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> m_StaticBlockEntities;
	float m_GroundLevel;
	int m_PreviousYLevel;
	float m_GravityFactor;
public:
	GravitationSystem(lua_State* L, std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> blocksEntities,
		std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>> staticBlocksEntities, float groundLevel)
		: m_L(L), m_BlockEntities(blocksEntities), m_StaticBlockEntities(m_StaticBlockEntities),
		m_GroundLevel(groundLevel), m_PreviousYLevel(0), m_GravityFactor(15.0f) {}

	bool OnUpdate(entt::registry& registry, float delta)
	{
		const auto gravityView = registry.view<Gravitation, Character, Health>();
		gravityView.each([&](entt::entity characterEntity, Gravitation& gravitation, Character& character, Health& characterHealth) {
			Vector3 blockPos =
			{
				int(character.Transform.Position.X / 3.0f),
				int((character.Transform.Position.Y - (character.Transform.Scale.Y / 2.0f)) / 3.0f) ,
				int(character.Transform.Position.Z / 3.0f)
			};

			if (blockPos.y < 0 && blockPos.y > -31 &&
				blockPos.x >= 0 && blockPos.x <= 14 && blockPos.z >= 0 && blockPos.z <= 14)
			{
				Block& blockUnder = registry.get<Block>((*m_BlockEntities)[size_t((blockPos.y * -1)) * size_t(225) + size_t(blockPos.z) * size_t(15) + size_t(blockPos.x)]);
				auto posUnder = blockUnder.Position;
				auto materialUnder = blockUnder.Material;
				if (blockPos.y < m_PreviousYLevel)
				{
					if (materialUnder != Materials::EMPTY && !gravitation.IsTouchingGround)
					{
						if (gravitation.VelocityY <= -10.0f)
						{
							characterHealth.CurrentHealth += 2.0f * (gravitation.VelocityY + 10.0f);
						}
						gravitation.IsTouchingGround = true;
						gravitation.VelocityY = 0;
						gravitation.GravityFactor = 0;
						character.Transform.Position.Y = blockPos.y * 3.0f + character.Transform.Scale.Y / 2.0f;
						m_PreviousYLevel = blockPos.y;
					}
				}
				else if (materialUnder == Materials::EMPTY && gravitation.IsTouchingGround == true)
				{
					gravitation.IsTouchingGround = false;
					gravitation.VelocityY = 0.0f;
					gravitation.GravityFactor = m_GravityFactor;
				}
			}
			else
			{
				if (blockPos.y == -1 && m_PreviousYLevel == 0 && !gravitation.IsTouchingGround)
				{
					if (gravitation.VelocityY <= -10.0f)
					{
						characterHealth.CurrentHealth += 2.0f * (gravitation.VelocityY + 10.0f);
					}
					gravitation.IsTouchingGround = true;
					gravitation.VelocityY = 0.0f;
					gravitation.GravityFactor = 0.0f;
					character.Transform.Position.Y = blockPos.y * 3.0f + character.Transform.Scale.Y / 2.0f;
					m_PreviousYLevel = blockPos.y;
					return false;
				}
			}
			m_PreviousYLevel = blockPos.y;
			if (!gravitation.IsTouchingGround)
			{
				character.Transform.Position.Y += gravitation.VelocityY * delta;
				gravitation = { gravitation.VelocityY - gravitation.GravityFactor * delta, gravitation.GravityFactor };

			}
			});

		return false;
	}
};

class CollisionSystem : public System
{
	lua_State* m_L;
	std::shared_ptr<Camera3D> m_Camera;
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> m_BlockEntities;
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>> m_StaticBlockEntities;
	std::vector<int> m_StaticBlocks;

	struct YRotatedCube
	{
		Vector3 CenterPos;
		Vector3 Dimensions;
		long double Angle;
	};
	struct Line
	{
		Vector2 Origin;
		Vector2 Direction;
	};
	struct Side
	{
		Line Line;
		Vector2 Normal;
	};
	struct CollisionInfo
	{
		Line ToProjected;
		Vector2 Normal;
		float Distance;
	};
private:
	bool CheckCollisionsCubeRotatedY(const YRotatedCube& cube1, const YRotatedCube& cube2) const
	{
		auto getRectAxes = [](const YRotatedCube& rectangle)->std::array<Line, 2>
		{
			const Vector2 rotatedXAxis{ Vector2Rotate({ 1.0f, 0.0f }, rectangle.Angle) };
			const Vector2 rotatedZAxis{ Vector2Rotate({ 0.0f, 1.0f }, rectangle.Angle) };
			const Vector2 rectangleCenter = { rectangle.CenterPos.x, rectangle.CenterPos.z };
			return
			{
				Line{ rectangleCenter, rotatedXAxis },
				Line{ rectangleCenter, rotatedZAxis }
			};
		};

		auto getCorners = [&](const YRotatedCube& rect)->std::array<Vector2, 4>
		{
			const auto [lineX, lineZ] = getRectAxes(rect);
			Vector2 offsetX{ Vector2Scale(lineX.Direction, rect.Dimensions.x / 2.0f) };
			Vector2 offsetZ{ Vector2Scale(lineZ.Direction, rect.Dimensions.z / 2.0f) };
			return
			{
				Vector2Add(Vector2Add({ rect.CenterPos.x, rect.CenterPos.z }, offsetX), offsetZ),
				Vector2Add(Vector2Add({ rect.CenterPos.x, rect.CenterPos.z }, offsetX), Vector2Scale(offsetZ, -1.0f)),
				Vector2Add(Vector2Add({ rect.CenterPos.x, rect.CenterPos.z }, Vector2Scale(offsetX, -1.0f)), Vector2Scale(offsetZ, -1.0f)),
				Vector2Add(Vector2Add({ rect.CenterPos.x, rect.CenterPos.z }, Vector2Scale(offsetX, -1.0f)), offsetZ)
			};
		};

		auto projectPointOnLine = [](const Vector2& point, const Line& line)->Vector2
		{
			const Vector2 fromLineOriginToPoint = Vector2Subtract(point, line.Origin);
			const float dotValue = Vector2DotProduct(line.Direction, fromLineOriginToPoint);
			return Vector2Add(line.Origin, Vector2Scale(line.Direction, dotValue));
		};

		// Check if rect's projection hits otherRect
		auto didBothProjectionsCollide = [&](const YRotatedCube& rect, const YRotatedCube& otherRect)->bool
		{
			bool collided = true;
			const auto axes = getRectAxes(otherRect);
			const auto corners = getCorners(rect);
			for (std::size_t i = 0; i < 2; ++i)
			{
				const float rectHalfSize = (i == 0 ? otherRect.Dimensions.x : otherRect.Dimensions.z) / 2.0f;
				float minSignedDistance = std::numeric_limits<float>::max();
				float maxSignedDistance = std::numeric_limits<float>::lowest();
				for (const auto& corner : corners)
				{
					const Vector2 projected{ projectPointOnLine(corner, axes[i]) };
					const Vector2 centerToProjected{ Vector2Subtract(projected, { otherRect.CenterPos.x, otherRect.CenterPos.z }) };
					const bool sign = Vector2DotProduct(centerToProjected, axes[i].Direction) > 0.0f;
					const float signedDistance = Vector2Length(centerToProjected) * (sign ? 1.0f : -1.0f);

					minSignedDistance = (minSignedDistance == std::numeric_limits<float>::max() || minSignedDistance > signedDistance)
						? signedDistance : minSignedDistance;
					maxSignedDistance = (maxSignedDistance == std::numeric_limits<float>::lowest() || maxSignedDistance < signedDistance)
						? signedDistance : maxSignedDistance;
				}

				if (!(minSignedDistance < 0.0f && maxSignedDistance > 0.0f ||
					(std::fabs(minSignedDistance) < rectHalfSize) || (std::fabs(maxSignedDistance) < rectHalfSize)))
				{
					return false;
				}
			}

			return collided;
		};

		if (std::fabs(cube1.CenterPos.y - cube2.CenterPos.y) > (cube1.Dimensions.y + cube2.Dimensions.y) / 2.0f)
			return false;

		const std::array corners1 = getCorners(cube1); // Player - cube1
		const std::array corners2 = getCorners(cube2);
		const std::array axes2 = getRectAxes(cube2);

		bool a = didBothProjectionsCollide(cube1, cube2);
		bool b = didBothProjectionsCollide(cube2, cube1);
		if (a && b)
		{
			const std::array sidesOfCube2 =
			{
				Side{{
						Vector2Add({ cube2.CenterPos.x, cube2.CenterPos.z },
									{ cube2.Dimensions.x / 2.0f, 0.0f }),
						axes2[1].Direction },
						{ 1.0f, 0.0f }
					},
				Side{{
						Vector2Subtract({ cube2.CenterPos.x, cube2.CenterPos.z },
										{ cube2.Dimensions.x / 2.0f, 0.0f }),
						axes2[1].Direction },
						{ -1.0f, 0.0f }
					},
				Side{{
						Vector2Add({ cube2.CenterPos.x, cube2.CenterPos.z },
									{ 0.0f, cube2.Dimensions.y / 2.0f }),
						axes2[0].Direction },
						{ 0.0f, 1.0f }
					},
				Side{{
						Vector2Subtract({ cube2.CenterPos.x, cube2.CenterPos.z },
										{ 0.0f, cube2.Dimensions.y / 2.0f }),
						axes2[0].Direction },
						{ 0.0f, -1.0f }
					}
			};

			std::vector<CollisionInfo> projectedLines;
			Vector2 normal{};
			projectedLines.reserve(16);
			for (const auto& side : sidesOfCube2)
			{
				const Line line = side.Line;
				const Vector3 origin{ side.Line.Origin.x, cube2.CenterPos.y, side.Line.Origin.y };
				for (const auto& corner : corners1)
				{
					const Vector2 projectedPoint = projectPointOnLine(corner, line);

					const auto collisionInfo = [&]() -> CollisionInfo
					{
						if (!(projectedPoint.x >= cube2.CenterPos.x - cube2.Dimensions.x / 2.0f &&
							projectedPoint.x <= cube2.CenterPos.x + cube2.Dimensions.x / 2.0f)
							|| !(projectedPoint.y >= cube2.CenterPos.z - cube2.Dimensions.z / 2.0f &&
								projectedPoint.y <= cube2.CenterPos.z + cube2.Dimensions.z / 2.0f))
						{
							const Vector2 firstCorner = Vector2Add(line.Origin, Vector2Scale(line.Direction, cube2.Dimensions.x / 2.0f));
							const Vector2 secondCorner = Vector2Add(line.Origin, Vector2Scale(line.Direction, cube2.Dimensions.x / -2.0f));

							const Vector2 cornerTofirstCorner = Vector2Subtract(firstCorner, corner);
							const Vector2 cornerToSecondCorner = Vector2Subtract(secondCorner, corner);

							return Vector2Length(cornerTofirstCorner) < Vector2Length(cornerToSecondCorner)
								? CollisionInfo{ { corner, cornerTofirstCorner }, side.Normal, Vector2Length(cornerTofirstCorner) }
							: CollisionInfo{ { corner, cornerTofirstCorner }, side.Normal, Vector2Length(cornerTofirstCorner) };
						}
						else
						{
							const Vector2 toProjected = Vector2Subtract(projectedPoint, corner);
							return CollisionInfo{ { corner, toProjected }, side.Normal, Vector2Length(toProjected) };
						}
					}();

					projectedLines.push_back(collisionInfo);
				}
			}

			const auto closestInfo = std::min_element(std::cbegin(projectedLines), std::cend(projectedLines),
				[](const CollisionInfo c1, const CollisionInfo c2) -> bool
				{
					return c1.Distance < c2.Distance;
				}
			);

			const Ray ray =
			{
				Vector3Add({ closestInfo->ToProjected.Origin.x, cube2.CenterPos.y, closestInfo->ToProjected.Origin.y },
				Vector3Negate(Vector3Scale({ closestInfo->ToProjected.Direction.x, 0.0f, closestInfo->ToProjected.Direction.y }, 1000.0f))),
				Vector3Normalize({ closestInfo->ToProjected.Direction.x, 0.0f, closestInfo->ToProjected.Direction.y })
			};
			const BoundingBox bb =
			{
				Vector3Subtract(cube2.CenterPos, Vector3Scale(cube2.Dimensions, 0.5f)),
				Vector3Add(cube2.CenterPos, Vector3Scale(cube2.Dimensions, 0.5f))
			};

			const auto rayCollision = GetRayCollisionBox(ray, bb);
			lua_getglobal(m_L, "collisionInfo");
			lua_pushboolean(m_L, true);
			lua_setfield(m_L, -2, "isColliding");
			lua_pushnumber(m_L, closestInfo->Distance);
			lua_setfield(m_L, -2, "distanceToCollision");
			lua_pushvector(m_L, { closestInfo->Normal.x, 0.0f, closestInfo->Normal.y });
			lua_setfield(m_L, -2, "normal");
			lua_pop(m_L, 1);

			return true;
		}

		return false;
	}
public:
	CollisionSystem(lua_State* L, std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> blockEntities,
		std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>> staticBlockEntities)
		: m_L(L), m_BlockEntities(blockEntities), m_StaticBlockEntities(staticBlockEntities)
	{
		m_StaticBlocks = { Materials::EMPTY, Materials::SURFACE_GRASS,
			Materials::UNBREAKABLE, Materials::FINAL_LEVEL_WALL, Materials::FINAL_LEVEL_GROUND };
	}

	bool OnUpdate(entt::registry& registry, float delta) final
	{
		const auto characterView = registry.view<Character>();
		const entt::entity characterEntityId = characterView.front();
		const LUATransform characterTransform = registry.get<Character>(characterEntityId).Transform;
		Vector3 blockPos =
		{
			int(characterTransform.Position.X / 3.0f),
			int((characterTransform.Position.Y) / 3.0f),
			int(characterTransform.Position.Z / 3.0f)
		};

		const LUAVector3 characterPosition = characterTransform.Position;
		const float halfWidth = characterTransform.Scale.X / 2.0f;
		const float halfHeight = characterTransform.Scale.Y / 2.0f;
		const BoundingBox characterBoundingBox = { { characterPosition.X - halfWidth, characterPosition.Y - halfHeight, characterPosition.Z - halfWidth },
													{ characterPosition.X + halfWidth, characterPosition.Y + halfHeight, characterPosition.Z + halfWidth } };

		if (blockPos.y < 0 && blockPos.y > -31)
		{
			for (const auto& staticBlockEntity : *m_StaticBlockEntities)
			{
				const Block& staticBlock = registry.get<Block>(staticBlockEntity);
				const LUAVector3 staticBlockPosition = staticBlock.Position;
				const LUAVector3 staticBlockSize = staticBlock.Size;
				if (CheckCollisionsCubeRotatedY({ { characterTransform.Position.X, characterTransform.Position.Y, characterTransform.Position.Z },
					   { characterTransform.Scale.X, characterTransform.Scale.Y, characterTransform.Scale.Z }, -std::atan2(long double(characterTransform.Rotation.X), long double(characterTransform.Rotation.Z)) },
					{ { staticBlockPosition.X, staticBlockPosition.Y, staticBlockPosition.Z }, { staticBlockSize.X, staticBlockSize.Y, staticBlockSize.Z }, 0.00 }))
				{
					return false;
				}
			}

			// If the player is in the air
			bool isTouchingGround = registry.get<Gravitation>(characterEntityId).IsTouchingGround;
			if (Gravitation* pGravitationComponent = registry.try_get<Gravitation>(characterEntityId);
				pGravitationComponent && !pGravitationComponent->IsTouchingGround)
			{
				auto start = m_BlockEntities->begin() + std::size_t(blockPos.y * -1) * std::size_t(225) - std::size_t(225);
				if (!(start >= m_BlockEntities->cbegin()))
					return false;

				for (auto it = start; it != start + std::size_t(255); it++)
				{
					Block& block = registry.get<Block>(*it);
					if (block.Material == Materials::EMPTY)
						continue;

					const LUAVector3 blockPosition = block.Position;
					const float blockSize = block.Size.X;
					if (CheckCollisionsCubeRotatedY({ { characterTransform.Position.X, characterTransform.Position.Y, characterTransform.Position.Z },
					   { characterTransform.Scale.X, characterTransform.Scale.Y, characterTransform.Scale.Z }, -std::atan2(long double(characterTransform.Rotation.X), long double(characterTransform.Rotation.Z)) },
						{ { blockPosition.X, blockPosition.Y, blockPosition.Z }, { blockSize, blockSize, blockSize }, 0.00 }))
					{
						Gravitation& gravitationComponent = registry.get<Gravitation>(characterEntityId);
						gravitationComponent.VelocityY = 0.0f;
						Character& characterComponent = registry.get<Character>(characterEntityId);
						characterComponent.Transform.Position.Y -= 0.005f;

						lua_getglobal(m_L, "collisionInfo");
						lua_pushboolean(m_L, true);
						lua_setfield(m_L, -2, "isColliding");
						lua_pushnumber(m_L, characterTransform.Position.Y - block.Position.Y - block.Size.Y / 2.0f);
						lua_setfield(m_L, -2, "distanceToCollision");
						lua_pushvector(m_L, { 0.0f, -1.0f, 0.0f });
						lua_setfield(m_L, -2, "normal");
						lua_pop(m_L, 1);
						return false;
					}
				}
			}

			// If the player is on ground
			auto start = m_BlockEntities->begin() + size_t(blockPos.y * -1) * size_t(225);
			for (auto it = start; it != start + std::size_t(225); it++)
			{
				Block& block = registry.get<Block>(*it);
				if (block.Material == Materials::EMPTY)
					continue;

				const LUAVector3 blockPosition = block.Position;
				const float blockSize = block.Size.X;
				if (CheckCollisionsCubeRotatedY({ { characterTransform.Position.X, characterTransform.Position.Y, characterTransform.Position.Z },
				   { characterTransform.Scale.X, characterTransform.Scale.Y, characterTransform.Scale.Z }, -std::atan2(long double(characterTransform.Rotation.X), long double(characterTransform.Rotation.Z)) },
					{ { blockPosition.X, blockPosition.Y, blockPosition.Z }, { blockSize, blockSize, blockSize }, 0.00 }))
				{
					return false;
				}
			}

			return false;
		}
		else if (blockPos.y <= -31)
		{
			for (const auto& staticBlockEntity : *m_StaticBlockEntities)
			{
				const Block& staticBlock = registry.get<Block>(staticBlockEntity);
				const LUAVector3 staticBlockPosition = staticBlock.Position;
				const LUAVector3 staticBlockSize = staticBlock.Size;
				if (CheckCollisionsCubeRotatedY({ { characterTransform.Position.X, characterTransform.Position.Y, characterTransform.Position.Z },
					   { characterTransform.Scale.X, characterTransform.Scale.Y, characterTransform.Scale.Z }, -std::atan2(long double(characterTransform.Rotation.X), long double(characterTransform.Rotation.Z)) },
					{ { staticBlockPosition.X, staticBlockPosition.Y, staticBlockPosition.Z }, { staticBlockSize.X, staticBlockSize.Y, staticBlockSize.Z }, 0.00 }))
				{
					return false;
				}
			}
		}
		return false;

		// OLD CODE BELOW

		// for (auto it = (*m_Blocks).cbegin(); it != (*m_Blocks).cbegin() + 1; ++it)
			// for (const auto& visibleBlock : *m_Blocks) // crbegin()
		for (auto it = m_BlockEntities->cbegin(); it != m_BlockEntities->cend(); ++it)
		{
			const Block& blockComponent = registry.get<Block>(*it);
			if (std::find(m_StaticBlocks.cbegin(), m_StaticBlocks.cend(), blockComponent.Material) != m_StaticBlocks.cend())
			{
				continue;
			}

			if (blockComponent.Material == Materials::EMPTY)
				continue;

			const Vector3 visibleBlockPosition = { blockComponent.Position.X, blockComponent.Position.Y, blockComponent.Position.Z };
			Block& block = registry.get<Block>(*it);
			const LUAVector3 blockPosition = block.Position;
			const float blockSize = block.Size.X;
			const float halfBlockSize = blockSize / 2.0f;
			const BoundingBox blockBoundingBox = {
			   { blockPosition.X - halfBlockSize, blockPosition.Y - halfBlockSize, blockPosition.Z - halfBlockSize },
			   { blockPosition.X + halfBlockSize, blockPosition.Y + halfBlockSize, blockPosition.Z + halfBlockSize } };

			CheckCollisionsCubeRotatedY({ { characterTransform.Position.X, characterTransform.Position.Y, characterTransform.Position.Z },
			   { characterTransform.Scale.X, characterTransform.Scale.Y, characterTransform.Scale.Z }, -std::atan2(long double(characterTransform.Rotation.X), long double(characterTransform.Rotation.Z)) },
				{ { blockPosition.X, blockPosition.Y, blockPosition.Z }, { blockSize, blockSize, blockSize }, 0.00 });
		}

		return false;
	}
};

using Vertex = Vector3;
using Triangle = Vertex[3];
class RendererSystem : public System
{
	lua_State* m_L;
	int m_ScreenWidth;
	int m_ScreenHeight;
	std::shared_ptr<Camera3D> m_Camera;
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> m_BlockEntities;
	std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>> m_StaticBlockEntities;
	struct YRotatedCube
	{
		Vector3 CenterPos;
		Vector3 Dimensions;
		long double Angle;
	};
	struct Line
	{
		Vector2 Origin;
		Vector2 Direction;
	};
	struct Side
	{
		Line Line;
		Vector2 Normal;
	};
	struct CollisionInfo
	{
		Line ToProjected;
		Vector2 Normal;
		float Distance;
	};
private:
	void DrawCubeRotatedY(const YRotatedCube& cubeRotatedY, const Color color = BLACK) const
	{
		const float x = cubeRotatedY.CenterPos.x;
		const float y = cubeRotatedY.CenterPos.y;
		const float z = cubeRotatedY.CenterPos.z;
		const float width = cubeRotatedY.Dimensions.x;
		const float height = cubeRotatedY.Dimensions.y;
		const float length = cubeRotatedY.Dimensions.z;

		auto rotateVertexY = [&](Vertex vertex, const float newAngle) -> Vertex {
			const Matrix MatT0 = MatrixTranslate(-x, -y, -z);
			const Matrix MatR0 = MatrixRotateY(newAngle);
			const Matrix MatT1 = MatrixTranslate(x, y, z);
			const Matrix MatRotation = MatrixMultiply(MatrixMultiply(MatT0, MatR0), MatT1);

			return
			{
				vertex.x * MatRotation.m0 + vertex.y * MatRotation.m4 + vertex.z * MatRotation.m8 + MatRotation.m12,
				vertex.x * MatRotation.m1 + vertex.y * MatRotation.m5 + vertex.z * MatRotation.m9 + MatRotation.m13,
				vertex.x * MatRotation.m2 + vertex.y * MatRotation.m6 + vertex.z * MatRotation.m10 + MatRotation.m14
			};
		};

		const float sAngle = cubeRotatedY.Angle;

		// Front face
		DrawTriangle3D(
			rotateVertexY({ (x - width / 2.0f), y - height / 2.0f, (z + length / 2.0f) }, sAngle),	// Bottom Left
			rotateVertexY({ (x + width / 2.0f), y - height / 2.0f, (z + length / 2.0f) }, sAngle),	// Bottom Right
			rotateVertexY({ (x - width / 2.0f), y + height / 2.0f, (z + length / 2.0f) }, sAngle),	// Top Left
			color
		);
		DrawTriangle3D(
			rotateVertexY({ x + width / 2.0f, y + height / 2.0f, z + length / 2.0f }, sAngle),  // Top Right
			rotateVertexY({ x - width / 2.0f, y + height / 2.0f, z + length / 2.0f }, sAngle),  // Top Left
			rotateVertexY({ x + width / 2.0f, y - height / 2.0f, z + length / 2.0f }, sAngle),  // Bottom Right
			color
		);

		// Back face
		DrawTriangle3D(
			rotateVertexY({ x - width / 2.0f, y - height / 2.0f, z - length / 2.0f }, sAngle),  // Bottom Left
			rotateVertexY({ x - width / 2.0f, y + height / 2.0f, z - length / 2.0f }, sAngle), // Top Left
			rotateVertexY({ x + width / 2.0f, y - height / 2.0f, z - length / 2.0f }, sAngle), // Bottom Right
			color
		);
		DrawTriangle3D(
			rotateVertexY({ x + width / 2.0f, y + height / 2.0f, z - length / 2.0f }, sAngle), // Top Right
			rotateVertexY({ x + width / 2.0f, y - height / 2.0f, z - length / 2.0f }, sAngle), // Bottom Right
			rotateVertexY({ x - width / 2.0f, y + height / 2.0f, z - length / 2.0f }, sAngle), // Top Left
			color
		);

		// Top face
		DrawTriangle3D(
			rotateVertexY({ x - width / 2.0f, y + height / 2.0f, z - length / 2.0f }, sAngle), // Top Left
			rotateVertexY({ x - width / 2.0f, y + height / 2.0f, z + length / 2.0f }, sAngle), // Bottom Left
			rotateVertexY({ x + width / 2.0f, y + height / 2.0f, z + length / 2.0f }, sAngle), // Bottom Right
			color
		);
		DrawTriangle3D(
			rotateVertexY({ x + width / 2.0f, y + height / 2.0f, z - length / 2.0f }, sAngle), // Top Right
			rotateVertexY({ x - width / 2.0f, y + height / 2.0f, z - length / 2.0f }, sAngle), // Top Left
			rotateVertexY({ x + width / 2.0f, y + height / 2.0f, z + length / 2.0f }, sAngle), // Bottom Right
			color
		);

		// Bottom face
		DrawTriangle3D(
			rotateVertexY({ x - width / 2.0f, y - height / 2.0f, z - length / 2.0f }, sAngle), // Top Left
			rotateVertexY({ x + width / 2.0f, y - height / 2.0f, z + length / 2.0f }, sAngle), // Bottom Right
			rotateVertexY({ x - width / 2.0f, y - height / 2.0f, z + length / 2.0f }, sAngle), // Bottom Left
			color
		);
		DrawTriangle3D(
			rotateVertexY({ x + width / 2.0f, y - height / 2.0f, z - length / 2.0f }, sAngle), // Top Right
			rotateVertexY({ x + width / 2.0f, y - height / 2.0f, z + length / 2.0f }, sAngle), // Bottom Right
			rotateVertexY({ x - width / 2.0f, y - height / 2.0f, z - length / 2.0f }, sAngle), // Top Left
			color
		);

		// Right face
		DrawTriangle3D(
			rotateVertexY({ x + width / 2.0f, y - height / 2.0f, z - length / 2.0f }, sAngle), // Bottom Right
			rotateVertexY({ x + width / 2.0f, y + height / 2.0f, z - length / 2.0f }, sAngle), // Top Right
			rotateVertexY({ x + width / 2.0f, y + height / 2.0f, z + length / 2.0f }, sAngle), // Top Left
			color
		);
		DrawTriangle3D(
			rotateVertexY({ x + width / 2.0f, y - height / 2.0f, z + length / 2.0f }, sAngle), // Bottom Left
			rotateVertexY({ x + width / 2.0f, y - height / 2.0f, z - length / 2.0f }, sAngle), // Bottom Right
			rotateVertexY({ x + width / 2.0f, y + height / 2.0f, z + length / 2.0f }, sAngle), // Top Left
			color
		);

		// Left face
		DrawTriangle3D(
			rotateVertexY({ x - width / 2.0f, y - height / 2.0f, z - length / 2.0f }, sAngle), // Bottom Right
			rotateVertexY({ x - width / 2.0f, y + height / 2.0f, z + length / 2.0f }, sAngle), // Top Left
			rotateVertexY({ x - width / 2.0f, y + height / 2.0f, z - length / 2.0f }, sAngle), // Top Right
			color
		);
		DrawTriangle3D(
			rotateVertexY({ x - width / 2.0f, y - height / 2.0f, z + length / 2.0f }, sAngle), // Bottom Left
			rotateVertexY({ x - width / 2.0f, y + height / 2.0f, z + length / 2.0f }, sAngle), // Top Left
			rotateVertexY({ x - width / 2.0f, y - height / 2.0f, z - length / 2.0f }, sAngle), // Bottom Right
			color
		);
	}
public:
	RendererSystem(lua_State* L, int screenWidth, int screenHeight, std::shared_ptr<std::array<entt::entity, NUMBER_OF_BLOCKS>> blockEntities,
		std::shared_ptr<std::array<entt::entity, NUMBER_OF_STATIC_BLOCKS>> staticBlockEntities, const std::shared_ptr<Camera3D>& camera)
		: m_ScreenWidth(screenWidth), m_ScreenHeight(screenHeight), m_L(L), m_BlockEntities(blockEntities), m_StaticBlockEntities(staticBlockEntities), m_Camera(camera)
	{
	}

	bool OnUpdate(entt::registry& registry, float delta) final
	{
		const auto characterView = registry.view<Character>();
		const entt::entity characterEntity = characterView.front();
		const Character& characterComponent = registry.get<Character>(characterEntity);
		const LUATransform characterTransform = characterComponent.Transform;
		const auto shopView = registry.view<Shop>();

		BeginDrawing();
		{
			ClearBackground(SKYBLUE);
			BeginMode3D(*m_Camera);
			{
				for (const auto& staticBlockEntity : *m_StaticBlockEntities)
				{
					const Block& blockToDraw = registry.get<Block>(staticBlockEntity);
					const Color blockColor = [&]()->Color
					{
						switch (blockToDraw.Material)
						{
						case Materials::STONE:
						{
							return GRAY;
						}
						case Materials::UNBREAKABLE:
						{
							return { 66, 66, 66, 255 };
						}
						case Materials::SURFACE_GRASS:
						{
							return DARKGREEN;
						}
						default:
							return BLACK;
						}
					}();

					DrawCube({ blockToDraw.Position.X, blockToDraw.Position.Y, blockToDraw.Position.Z },
						blockToDraw.Size.X, blockToDraw.Size.Y, blockToDraw.Size.Z, blockColor);
				}


				for (const auto& blockEntity : *m_BlockEntities)
				{
					const Block& blockToDraw = registry.get<Block>(blockEntity);
					if (blockToDraw.Material == Materials::EMPTY)
						continue;

					const Color blockColor = [&]() -> Color
					{
						switch (blockToDraw.Material)
						{
						case Materials::GRASS:
						{
							return { 86, 125, 70, 255 };
						}
						case Materials::DIRT:
						{
							return DARKBROWN;
						}
						case Materials::STONE:
						{
							return GRAY;
						}
						case Materials::BRONZE:
						{
							return { 205, 127, 50, 255 };
						}
						case Materials::SILVER:
						{
							return { 192, 192, 192, 255 };
						}
						case Materials::GOLDEN:
						{
							return GOLD;
						}
						case Materials::DIAMOND:
						{
							return SKYBLUE;
						}
						case Materials::ANCIENT_RELIC:
						{
							return BEIGE;
						}
						default:
							return DARKBROWN;
						}
					}();

					DrawCube({ blockToDraw.Position.X, blockToDraw.Position.Y, blockToDraw.Position.Z },
						blockToDraw.Size.X, blockToDraw.Size.Y, blockToDraw.Size.Z, blockColor);
					DrawCubeWires({ blockToDraw.Position.X, blockToDraw.Position.Y, blockToDraw.Position.Z }, blockToDraw.Size.X, blockToDraw.Size.Y, blockToDraw.Size.Z, MAROON);
				}

				const auto moleView = registry.view<Mole>();
				moleView.each([&](Mole& moleComponent)
					{
						DrawCube({ moleComponent.Transform.Position.X,  moleComponent.Transform.Position.Y , moleComponent.Transform.Position.Z },
							moleComponent.Transform.Scale.X, moleComponent.Transform.Scale.Y, moleComponent.Transform.Scale.Z, BLUE);

						const YRotatedCube durabilityLeft =
						{
							{ moleComponent.Transform.Position.X, moleComponent.Transform.Position.Y + moleComponent.Transform.Scale.Y / 2.0f + 0.6f, moleComponent.Transform.Position.Z },
							Vector3Scale({moleComponent.Health / moleComponent.MaxHealth * moleComponent.Transform.Scale.X, 0.1f, 0.05f}, 1.0f),
							-std::atan2(long double(characterTransform.Rotation.X), long double(characterTransform.Rotation.Z))
						};

						if (moleComponent.MaxHealth == 150)
							DrawCubeRotatedY(durabilityLeft, GREEN);
						else if (moleComponent.MaxHealth == 600)
							DrawCubeRotatedY(durabilityLeft, YELLOW);
						else if (moleComponent.MaxHealth == 1350)
							DrawCubeRotatedY(durabilityLeft, ORANGE);
						else
							DrawCubeRotatedY(durabilityLeft, RED);
					});

				const auto treasureView = registry.view<Treasure>();
				treasureView.each([&](Treasure& treasure)
					{
						DrawCube({ treasure.Transform.Position.X, treasure.Transform.Position.Y, treasure.Transform.Position.Z },
							treasure.Transform.Scale.X, treasure.Transform.Scale.Y, treasure.Transform.Scale.Z, BLACK);

						const YRotatedCube durabilityLeft =
						{
							{treasure.Transform.Position.X, treasure.Transform.Position.Y + treasure.Transform.Scale.Y / 2.0f + 0.6f, treasure.Transform.Position.Z},
							Vector3Scale({treasure.Durability / treasure.MaxDurability * treasure.Transform.Scale.X, 0.1f, 0.05f}, 1.0f),
							-std::atan2(long double(characterTransform.Rotation.X), long double(characterTransform.Rotation.Z))
						};
						DrawCubeRotatedY(durabilityLeft, GREEN);
					});

				shopView.each([](const Shop& shop)
					{
						DrawCube({ shop.Transform.Position.X, shop.Transform.Position.Y, shop.Transform.Position.Z },
							shop.Transform.Scale.X, shop.Transform.Scale.Y, shop.Transform.Scale.Z, { unsigned char(shop.Color.X),
								unsigned char(shop.Color.Y), unsigned char(shop.Color.Z), unsigned char(255) });
					});
			}
			EndMode3D();

			std::unique_ptr<std::pair<const Shop, const entt::entity>> openShop = nullptr;
			shopView.each([&](const entt::entity shopId, const Shop& shop)
				{
					if (shop.ShopOpen)
						openShop = std::make_unique<std::pair<const Shop, const entt::entity>>(shop, shopId);
				});

			// Draw health 
			const auto healthView = registry.view<Health>();
			const Health& health = registry.get<Health>(healthView.front());
			const int posX = m_ScreenWidth * 0.05;
			const int posY = m_ScreenHeight * 0.05;
			const int width = m_ScreenWidth * 0.2;
			const int height = m_ScreenHeight * 0.07;
			DrawRectangle(posX, posY, width, height, RED);
			DrawRectangle(posX, posY, width * (health.CurrentHealth / health.MaxHealth), height, GREEN);
			DrawText(std::to_string(int(health.CurrentHealth)).c_str(), posX + width / 2.0, height, 30, BLACK);

			// Draw fuel 
			const auto fuelView = registry.view<Fuel>();
			const Fuel& fuel = registry.get<Fuel>(fuelView.front());
			DrawRectangle(posX, posY + height * 1.5, width, height, GRAY);
			DrawRectangle(posX, posY + height * 1.5, width * (fuel.CurrentFuel / fuel.MaxFuel), height, BROWN);
			const char* fuelText = (fuel.CurrentFuel >= 0) ? (std::to_string(int(fuel.CurrentFuel)).c_str()) : "???";
			if (fuel.CurrentFuel >= 0)
				DrawText(std::to_string(int(fuel.CurrentFuel)).c_str(), posX + width / 2.0, posY + height * 1.75, 30, BLACK);
			else
				DrawText("??????", posX + width / 3.0, posY + height * 1.75, 30, BLACK);

			const Backpack& backpackComponent = registry.get<Backpack>(characterEntity);
			DrawText(std::string{ "Backpack: " + std::to_string(int(backpackComponent.CurrentWeight)) + "/" + std::to_string(int(backpackComponent.MaxWeight)) }.c_str(),
				m_ScreenWidth * 0.75, m_ScreenHeight * 0.05, 30, WHITE);

			// Draw character gold 
			DrawText(("Gold: " + std::to_string(int(characterComponent.Gold))).c_str(), m_ScreenWidth * 0.45, m_ScreenHeight * 0.05, 30, GOLD);

			if (openShop)
			{
				EnableCursor();

				const Shop shopComponent = openShop->first;
				const entt::entity shopId = openShop->second;

				lua_rawgeti(m_L, LUA_REGISTRYINDEX, shopComponent.OnOpenRef);
				lua_pcall(m_L, 0, 0, 0);

				DrawRectangle((shopComponent.ScreenPointX * m_ScreenWidth) - (shopComponent.ScreenWidth * m_ScreenWidth) / 2,
					(shopComponent.ScreenPointY * m_ScreenHeight) - (shopComponent.ScreenHeight * m_ScreenHeight) / 2,
					shopComponent.ScreenWidth * m_ScreenWidth, shopComponent.ScreenHeight * m_ScreenHeight, WHITE);
				DrawText(shopComponent.Name, (shopComponent.ScreenPointX * m_ScreenWidth) + shopComponent.ScreenWidth / 2 - 50,
					(shopComponent.ScreenPointY * m_ScreenHeight) - (shopComponent.ScreenHeight * m_ScreenHeight) / 2, 25, BLACK);

				const Vector2 mouse = GetMousePosition();

				const auto buttonView = registry.view<Button>();
				std::unique_ptr<Button> hoverAboveButton = nullptr;
				buttonView.each([&](const entt::entity buttonId, const Button button)
					{
						if (entt::entity(button.ShopId) == shopId)
						{
							const float newFrameX = (shopComponent.ScreenPointX * m_ScreenWidth) - (shopComponent.ScreenWidth * m_ScreenWidth) / 2.0f;
							const float newFrameY = (shopComponent.ScreenPointY * m_ScreenHeight) - (shopComponent.ScreenHeight * m_ScreenHeight) / 2.0f;
							const float newX = (newFrameX + button.ScreenPointX * shopComponent.ScreenWidth * m_ScreenWidth) - (button.ScreenWidth * m_ScreenWidth) / 2.0f;
							const float newY = (newFrameY + button.ScreenPointY * shopComponent.ScreenHeight * m_ScreenHeight) - button.ScreenHeight * m_ScreenHeight;
							const Rectangle rec = { newX, newY, button.ScreenWidth * m_ScreenWidth, button.ScreenHeight * m_ScreenHeight };
							const bool isPressed = GuiButton(rec, button.Name);

							if (CheckCollisionPointRec(mouse, rec))
							{
								hoverAboveButton = std::make_unique<Button>(button);
							}

							if (isPressed)
							{
								lua_rawgeti(m_L, LUA_REGISTRYINDEX, button.OnClickRef);
								lua_pcall(m_L, 0, 0, 0);
							}
						}
					});

				if (hoverAboveButton)
				{
					Rectangle toolTip = { mouse.x + 90.0f, mouse.y - 25.0f, hoverAboveButton->ScreenWidth * m_ScreenWidth, hoverAboveButton->ScreenHeight * m_ScreenHeight };
					DrawLine(mouse.x, mouse.y, mouse.x + 50, mouse.y - 12.5, BLACK);
					DrawRectangle(mouse.x + 50, mouse.y - 25, hoverAboveButton->ScreenWidth * m_ScreenWidth, hoverAboveButton->ScreenHeight * m_ScreenHeight, GREEN);
					DrawRectangleLines(mouse.x + 50, mouse.y - 25, hoverAboveButton->ScreenWidth * m_ScreenWidth, hoverAboveButton->ScreenHeight * m_ScreenHeight, BLACK);
					lua_rawgeti(m_L, LUA_REGISTRYINDEX, hoverAboveButton->Tooltip);
					lua_pcall(m_L, 0, 1, 0);
					const char* message = lua_tostring(m_L, -1);
					lua_pop(m_L, 1);
					GuiLabelButton(toolTip, message);
				}

				const auto textLabelView = registry.view<TextLabel>();
				textLabelView.each([&](const entt::entity textLabelId, const TextLabel textLabel)
					{
						if (entt::entity(textLabel.ShopId) == shopId)
						{
							const float newFrameX = (shopComponent.ScreenPointX * m_ScreenWidth) - (shopComponent.ScreenWidth * m_ScreenWidth) / 2.0f;
							const float newFrameY = (shopComponent.ScreenPointY * m_ScreenHeight) - (shopComponent.ScreenHeight * m_ScreenHeight) / 2.0f;
							const float newX = (newFrameX + textLabel.ScreenPointX * shopComponent.ScreenWidth * m_ScreenWidth) - (textLabel.ScreenWidth * m_ScreenWidth) / 2.0f;
							const float newY = (newFrameY + textLabel.ScreenPointY * shopComponent.ScreenHeight * m_ScreenHeight) - textLabel.ScreenHeight * m_ScreenHeight;
							const Rectangle rec = { newX, newY, textLabel.ScreenWidth * m_ScreenWidth, textLabel.ScreenHeight * m_ScreenHeight };

							GuiLabel(rec, textLabel.Name);
						}
					});
			}
			else
			{
				DisableCursor();
				DrawCircleV({ m_ScreenWidth / 2.0f, m_ScreenHeight / 2.0f }, 3.0f, WHITE);
			}

			DrawFPS(m_ScreenWidth - 10.0f, 10.0f);
			DrawText(TextFormat("Character (X,Y,Z): %f, %f, %f", characterTransform.Position.X, characterTransform.Position.Y, characterTransform.Position.Z), 10, 10, 20, DARKBROWN);
		}

		EndDrawing();
		return false;
	}
};
#pragma once
#include "Tako.hpp"
#include "World.hpp"
#include "Grid.hpp"
#include "Player.hpp"
#include "Renderer.hpp"
#include "parson.h"
#include "Entities.hpp"
#include "DialogSystem.hpp"

class Room
{
public:

	void Init(std::string levelName, std::function<tako::Texture*(std::string)> getTexture)
	{
		std::string levelFolder = "/Map/" + levelName + "/";
		std::string file = levelFolder + "data.json";
		constexpr size_t bufferSize = 1024 * 1024;
		std::array <tako::U8, bufferSize> buffer;
		size_t bytesRead = 0;
		if (!tako::FileSystem::ReadFile(file.c_str(), buffer.data(), bufferSize, bytesRead))
		{
			LOG_ERR("Could not read level {}", file);
		}
		buffer[bytesRead] = '/0';

		auto levelJson = json_value_get_object(json_parse_string((char*)buffer.data()));

		auto layerArray = json_object_get_array(levelJson, "layers");
		auto layerCount = json_array_get_count(layerArray);

		for (int i = 0; i < layerCount; i++)
		{
			auto layer = json_array_get_string(layerArray, i);
			m_tilesPNG.push_back(getTexture(levelFolder + layer));
		}

		auto intGrid = tako::Bitmap::FromFile((levelFolder + "IntGrid-int.png").c_str());
		for (int x = 0; x < intGrid.Width(); x++)
		{
			for (int y = 0; y < intGrid.Height(); y++)
			{
				auto p = intGrid.GetPixel(x, y);
				if (p == tako::Color(0, 0, 0, 255))
				{
					m_world.Create(
						Tile{x, -y}
					);
				}
			}
		}

		auto entitiesJSON = json_object_get_object(levelJson, "entities");
		auto loadEntityType = [&](const char* name, std::function<void(int, int)> callback)
		{
			auto array = json_object_get_array(entitiesJSON, name);
			auto count = json_array_get_count(array);
			for (int i = 0; i < count; i++)
			{
				auto jsonObj = json_array_get_object(array, i);
				int x = json_object_get_number(jsonObj, "x") / 8;
				int y = -json_object_get_number(jsonObj, "y") / 8;
				callback(x, y);
			}
		};

		loadEntityType("PlayerSpawn", [&](int x, int y)
		{
			m_world.Create
			(
				Player(),
				Camera(),
				GridObject(x, y),
				MovingObject{x, y, 5},
				SpriteRenderer{{8,8}, getTexture("/Player.png"), {0, 0}}
			);
		});

		loadEntityType("Dog", [&](int x, int y)
		{
			m_world.Create
			(
				GridObject(x, y),
				MovingObject{x, y, 2},
				Dog(),
				SpriteRenderer{{8,8}, getTexture("/Dog.png"), {0, 0}},
				Interactable{[=](const tako::World& world, tako::Entity self, tako::Entity other)
				{
					DialogSystem::StartDialog(
					{
						"Oh, who is a good boy?!\nYes you are!",
						"Wuff, GRRR!"
					});
					return true;
				}}
			);
		});


	}

	void Update(tako::Input* input, float dt)
	{
		m_world.IterateComps<tako::Entity, GridObject, MovingObject, Player>([&](tako::Entity ent, GridObject& grid, MovingObject& move, Player& player)
		{
			if (!DialogSystem::IsOpen())
			{
				if (!move.IsMoving(grid))
				{
					if (input->GetKeyDown(tako::Key::L) || input->GetKeyDown(tako::Key::Gamepad_A))
					{
						auto targetTile = grid.GetTile() + GetFaceDelta(player.facing);
						bool interacted = false;
						m_world.IterateComps<tako::Entity, GridObject, Interactable>([&](tako::Entity e, GridObject& g, Interactable& inter)
						{
							if (!interacted && g.GetTile() == targetTile)
							{
								interacted = inter.callback(m_world, e, ent);
							}
						});
					}
					else
					{
						//TODO: limit movement to one direction at a time
						int xDelta = 0;
						int yDelta = 0;
						if (input->GetKey(tako::Key::Left) || input->GetKey(tako::Key::A) || input->GetKey(tako::Key::Gamepad_Dpad_Left))
						{
							xDelta -= 1;
						}
						if (input->GetKey(tako::Key::Right) || input->GetKey(tako::Key::D) || input->GetKey(tako::Key::Gamepad_Dpad_Right))
						{
							xDelta += 1;
						}
						if (input->GetKey(tako::Key::Up) || input->GetKey(tako::Key::W) || input->GetKey(tako::Key::Gamepad_Dpad_Up))
						{
							yDelta += 1;
						}
						if (input->GetKey(tako::Key::Down) || input->GetKey(tako::Key::S) || input->GetKey(tako::Key::Gamepad_Dpad_Down))
						{
							yDelta -= 1;
						}
						if (xDelta != 0 && yDelta != 0)
						{
							if (player.facing == FaceDirection::Down || player.facing == FaceDirection::Up)
							{
								yDelta = 0;
							}
							else
							{
								xDelta = 0;
							}
						}
						if (xDelta != 0 || yDelta != 0)
						{
							if (xDelta == 1)
							{
								player.facing = FaceDirection::Right;
							}
							if (xDelta == -1)
							{
								player.facing = FaceDirection::Left;
							}
							if (yDelta == 1)
							{
								player.facing = FaceDirection::Up;
							}
							if (yDelta == -1)
							{
								player.facing = FaceDirection::Down;
							}
							Grid::Move(ent, xDelta, yDelta, m_world);
						}
					}
				}
			}
		});

		m_world.IterateComps<tako::Entity, GridObject, MovingObject, Dog>([&](tako::Entity ent, GridObject& grid, MovingObject& move, Dog& dog)
		{
			if (!move.IsMoving(grid) && rand() % 250 == 1)
			{
				bool mvX = rand() % 2;
				int sign = rand() % 2 ? 1 : -1;
				Grid::Move(ent, mvX ? sign : 0, mvX ? 0 : sign, m_world);
			}
		});
		m_world.IterateComps<GridObject, MovingObject>([&](GridObject& grid, MovingObject& move)
		{
			if (move.targetX != grid.x)
			{
				grid.x += (move.targetX - grid.x) * move.gridSpeed * dt;
				if (tako::mathf::abs(grid.x - move.targetX) < EPSILON)
				{
					grid.x = move.targetX;
				}
			}
			if (move.targetY != grid.y)
			{
				grid.y += (move.targetY - grid.y) * move.gridSpeed * dt;
				if (tako::mathf::abs(grid.y - move.targetY) < EPSILON)
				{
					grid.y = move.targetY;
				}
			}
		});
	}

	void Draw(tako::OpenGLPixelArtDrawer* drawer, PaletteSprite* m_tile)
	{
		drawer->SetCameraPosition(GetCamera());

		for (auto layer : m_tilesPNG)
		{
			drawer->DrawImage(0, 0, layer->width, layer->height, layer->handle);
		}

		m_world.IterateComps<GridObject, RectangleRenderer>([&](GridObject& grid, RectangleRenderer& rect)
		{
			drawer->DrawRectangle(grid.x * 8, grid.y * 8, rect.size.x, rect.size.y, rect.color);
		});

		m_world.IterateComps<GridObject, SpriteRenderer>([&](GridObject& grid, SpriteRenderer& sp)
		{
			drawer->DrawImage(grid.x * 8, grid.y * 8, sp.size.x, sp.size.y, sp.sprite->handle);
		});
	}


private:
	tako::World m_world;
	std::vector<tako::Texture*> m_tilesPNG;

	tako::Vector2 GetCamera()
	{
		tako::Vector2 pos;
		m_world.IterateComps<GridObject, Camera>([&](GridObject& grid, Camera& cam)
		{
			pos = { grid.x * 8, grid.y * 8};
		});

		return pos;
	}
};
#pragma once
#include "Tako.hpp"
#include "World.hpp"
#include "Grid.hpp"
#include "Player.hpp"
#include "Renderer.hpp"

class Room
{
public:

	void Init(std::function<tako::Texture*(std::string)> getTexture)
	{
		m_world.Create
		(
			Player(),
			Camera(),
			GridObject(0, 0),
			MovingObject{0, 0, 5},
			SpriteRenderer{{8,8}, getTexture("/Player.png"), {0, 0}}
		);
		m_world.Create
		(
			GridObject(-2, 0),
			SpriteRenderer{{8,8}, getTexture("/Dog.png"), {0, 0}}
		);
	}

	void Update(tako::Input* input, float dt)
	{
		m_world.IterateComps<tako::Entity, GridObject, MovingObject, Player>([&](tako::Entity ent, GridObject& grid, MovingObject& move, Player& player)
		{
			if (!move.IsMoving(grid))
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
				Grid::Move(ent, xDelta, yDelta, m_world);
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
		for (int x = -10; x < 10; x++)
		{
			for (int y = -10; y < 10; y++)
			{
				drawer->DrawImage(x * 8, y * 8, 8, 8, m_tile->texture.handle);
			}
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
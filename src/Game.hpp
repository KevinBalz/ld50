#pragma once
#include "Tako.hpp"
#include "OpenGLPixelArtDrawer.hpp"
#include "World.hpp"
#include "Renderer.hpp"
#include "GridObject.hpp"
#include "Player.hpp"

class Game
{
public:
	void Setup(const tako::SetupData& setup)
	{
		m_context = setup.context;
		m_drawer = new tako::OpenGLPixelArtDrawer(setup.context);
		m_drawer->Resize(1024, 768);
		m_drawer->SetTargetSize(240, 135);
		m_drawer->AutoScale();

		InitWorld();
	}

	void InitWorld()
	{
		m_world.Create
		(
			Player(),
			GridObject(0, 0),
			MovingObject{0, 0, 5},
			RectangleRenderer{{8,8}, {255, 0, 0, 255}}
		);
	}

	void Update(tako::Input* input, float dt)
	{
		m_world.IterateComps<GridObject, MovingObject, Player>([&](GridObject& grid, MovingObject& move, Player& player)
		{
			if (!move.IsMoving(grid))
			{
				if (input->GetKey(tako::Key::Left) || input->GetKey(tako::Key::A) || input->GetKey(tako::Key::Gamepad_Dpad_Left))
				{
					move.targetX -= 1;
				}
				if (input->GetKey(tako::Key::Right) || input->GetKey(tako::Key::D) || input->GetKey(tako::Key::Gamepad_Dpad_Right))
				{
					move.targetX += 1;
				}
				if (input->GetKey(tako::Key::Up) || input->GetKey(tako::Key::W) || input->GetKey(tako::Key::Gamepad_Dpad_Up))
				{
					move.targetY += 1;
				}
				if (input->GetKey(tako::Key::Down) || input->GetKey(tako::Key::S) || input->GetKey(tako::Key::Gamepad_Dpad_Down))
				{
					move.targetY -= 1;
				}
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

	void Draw()
	{
		auto drawer = m_drawer;
		m_context->Begin();
		drawer->Clear();

		m_world.IterateComps<GridObject, RectangleRenderer>([&](GridObject& grid, RectangleRenderer& rect)
		{
			drawer->DrawRectangle(grid.x * 8, grid.y * 8, rect.size.x, rect.size.y, rect.color);
		});

		m_context->End();
	}
private:
	tako::GraphicsContext* m_context = nullptr;
	tako::OpenGLPixelArtDrawer* m_drawer;
	tako::World m_world;
};



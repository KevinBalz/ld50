#pragma once
#include "Tako.hpp"
#include "OpenGLPixelArtDrawer.hpp"
#include "World.hpp"
#include "Renderer.hpp"
#include "Grid.hpp"
#include "Player.hpp"
#include <time.h>
#include <stdlib.h>

class Game
{
public:
	void Setup(const tako::SetupData& setup)
	{
		srand(time(NULL));
		m_context = setup.context;
		m_drawer = new tako::OpenGLPixelArtDrawer(setup.context);
		m_drawer->Resize(1024, 768);
		m_drawer->SetTargetSize(240, 135);
		m_drawer->AutoScale();

		ChangePalette(DefaultPalette);
		m_tile = LoadPaletteSprite("/Tile.png");

		InitWorld();
	}

	tako::Bitmap ApplyPaletteBitmap(const tako::Bitmap& bitmap, const Palette& palette)
	{
		auto b = bitmap.Clone();
		for (int x = 0; x < b.Width(); x++)
		{
			for (int y = 0; y < b.Height(); y++)
			{
				auto pixel = b.GetPixel(x, y);
				if (pixel.a == 0)
				{
					continue;
				}
				switch (pixel.r)
				{
					case   0: b.SetPixel(x,y, palette[0]); break;
					case  85: b.SetPixel(x,y, palette[1]); break;
					case 170: b.SetPixel(x,y, palette[2]); break;
					case 255: b.SetPixel(x,y, palette[3]); break;
				}
			}
		}
		return std::move(b);
	}

	PaletteSprite* LoadPaletteSprite(std::string path)
	{
		auto bitmap = tako::Bitmap::FromFile(path.c_str());
		auto paletted = ApplyPaletteBitmap(bitmap, m_activePalette);
		m_spriteMap[path] = { std::move(bitmap), m_drawer->CreateTexture(paletted)};
		return &m_spriteMap[path];
	}

	void ChangePalette(const Palette& palette)
	{
		m_activePalette = palette;
		for (auto& [key, sprite] : m_spriteMap)
		{
			auto bitmap = ApplyPaletteBitmap(sprite.source, palette);
			m_drawer->UpdateTexture(sprite.texture, bitmap);
		}
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

	tako::Color RandomColor()
	{
		return tako::Color(rand() % 255, rand() % 255, rand() % 255, 255);
	}

	void Update(tako::Input* input, float dt)
	{
		if (input->GetKeyDown(tako::Key::Space))
		{
			ChangePalette({RandomColor(),RandomColor(),RandomColor(),RandomColor()});
		}
		m_world.IterateComps<GridObject, MovingObject, Player>([&](GridObject& grid, MovingObject& move, Player& player)
		{
			if (!move.IsMoving(grid))
			{
				//TODO: limit movement to one direction at a time
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
		m_drawer->SetClearColor(m_activePalette[3]);

		for (int x = -50; x < 50; x++)
		{
			for (int y = -50; y < 50; y++)
			{
				drawer->DrawImage(x * 8, y * 8, 8, 8, m_tile->texture.handle);
			}
		}

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
	std::unordered_map<std::string, PaletteSprite> m_spriteMap;
	PaletteSprite* m_tile;
	Palette m_activePalette;
};



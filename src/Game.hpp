#pragma once
#include "Tako.hpp"
#include "OpenGLPixelArtDrawer.hpp"
#include "World.hpp"
#include "Renderer.hpp"
#include "Grid.hpp"
#include "Player.hpp"
#include <time.h>
#include <stdlib.h>
#include "Room.hpp"
#include "Routine.hpp"

class Game
{
public:
	void Setup(const tako::SetupData& setup)
	{
		srand(time(NULL));
		m_context = setup.context;
		m_drawer = new tako::OpenGLPixelArtDrawer(setup.context);
		m_drawer->Resize(1200, 675);
		m_drawer->SetTargetSize(240/2, 135/2);
		m_drawer->AutoScale();

		Routine::Init();
		Diary::Init();
		Diary::Reset();
		DialogSystem::Init(m_drawer);

		DialogSystem::NightTimeReading(Diary::GetNightTimeReading(), [=]() mutable
		{
			Diary::StartNextDay();
			m_activeRoom = Room();
			m_activeRoom->Init("House", [&](std::string path)
			{
				auto paletteSprite = LoadPaletteSprite(path);
				return &paletteSprite->texture;
			});
		});
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
		auto search = m_spriteMap.find(path);
		if (search != m_spriteMap.end())
		{
			return &search->second;
		}
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

	void Update(tako::Input* input, float dt)
	{
		Routine::Update(input, dt);
		DialogSystem::Update(input, dt);
		if (m_activeRoom)
		{
			m_activeRoom->Update(input, dt);
		}
	}

	void Draw()
	{
		auto drawer = m_drawer;
		if (m_activePalette != PaletteManager::Get())
		{
			ChangePalette(PaletteManager::Get());
		}
		m_context->Begin();
		m_drawer->SetClearColor(m_activePalette[3]);
		drawer->Clear();

		if (m_activeRoom)
		{
			m_activeRoom->Draw(drawer, m_activePalette);
		}
		DialogSystem::Draw(m_drawer, m_activePalette);

		m_context->End();
	}
private:
	tako::GraphicsContext* m_context = nullptr;
	tako::OpenGLPixelArtDrawer* m_drawer;
	std::unordered_map<std::string, PaletteSprite> m_spriteMap;
	Palette m_activePalette;
	std::optional<Room> m_activeRoom;
};



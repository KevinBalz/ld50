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
#include "SceneManager.hpp"

bool GetAnyDown(tako::Input* input)
{
	for (int i = 0; i < (int) tako::Key::Unknown; i++)
	{
		if (input->GetKeyDown((tako::Key) i))
		{
			return true;
		}
	}

	return false;
}

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

		m_font = new tako::Font("/charmap-cellphone.png", 5, 7, 1, 1, 2, 2,
								" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]\a_`abcdefghijklmnopqrstuvwxyz{|}~");

		Routine::Init();
		Diary::Init();
		Diary::Reset();
		DialogSystem::Init(m_drawer, m_font);

		SceneManager::Init
		(
			[=]()
			{
				Diary::Reset();
				DialogSystem::NightTimeReading(Diary::GetNightTimeReading(), [=]()
				{
					Diary::StartNextDay();
					SceneManager::LoadRoom("House");
				});
			},
			[=](auto name)
			{
				m_activeRoom = Room();
				m_activeRoom->Init(name, [=](std::string path)
				{
					auto paletteSprite = LoadPaletteSprite(path);
					return &paletteSprite->texture;
				});
			}
		);

		{
			auto bitmap = m_font->RenderText("Press a button to start", 1);
			m_textPressAny = m_drawer->CreateTexture(bitmap);
		}
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
		if (m_pressAny)
		{
			if (GetAnyDown(input))
			{
				m_pressAny = false;
				//TODO: starting stuff for title
			}
			return;
		}
		if (m_showTitle)
		{
			if (GetAnyDown(input))
			{
				m_showTitle = false;
				SceneManager::StartBeginning();
				//TODO: starting stuff for title
			}
			return;
		}
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
		if (m_pressAny)
		{
			drawer->SetTargetSize(240, 135);
			m_drawer->SetClearColor(tako::Color("#000000"));
			drawer->Clear();
			drawer->SetCameraPosition({0, 0});
			drawer->DrawImage(-m_textPressAny.width/2, m_textPressAny.height/2, m_textPressAny.width, m_textPressAny.height, m_textPressAny.handle);
		}
		else if (m_showTitle)
		{
			drawer->SetTargetSize(240, 135);
			m_drawer->SetClearColor(m_activePalette[3]);
			drawer->Clear();
		}
		else
		{
			m_drawer->SetClearColor(m_activePalette[3]);
			drawer->Clear();

			if (m_activeRoom)
			{
				m_activeRoom->Draw(drawer, m_activePalette);
			}
			DialogSystem::Draw(m_drawer, m_activePalette);
		}


		m_context->End();
	}
private:
	bool m_pressAny = true;
	bool m_showTitle = true;
	tako::GraphicsContext* m_context = nullptr;
	tako::OpenGLPixelArtDrawer* m_drawer;
	std::unordered_map<std::string, PaletteSprite> m_spriteMap;
	Palette m_activePalette;
	std::optional<Room> m_activeRoom;

	tako::Font* m_font;
	tako::Texture m_textPressAny;
};



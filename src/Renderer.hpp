#pragma once
#include "Tako.hpp"

using Palette = std::array<tako::Color, 4>;

constexpr Palette DefaultPalette = {tako::Color("#081820"), tako::Color("#346856"), tako::Color("#88c070"), tako::Color("#e0f8d0")};

class PaletteManager
{
public:

	static void Set(const Palette& pal)
	{
		Instance().m_active = pal;
	}

	static void Reset()
	{
		Instance().m_active = DefaultPalette;
	}

	static void Random()
	{
		Set
		({
			tako::Color(rand() % 255, rand() % 255, rand() % 255, 255),
			tako::Color(rand() % 255, rand() % 255, rand() % 255, 255),
			tako::Color(rand() % 255, rand() % 255, rand() % 255, 255),
			tako::Color(rand() % 255, rand() % 255, rand() % 255, 255)
		});
	}

	static void Black()
	{
		Set
		({
			tako::Color("#000000"),
			tako::Color("#000000"),
			tako::Color("#000000"),
			tako::Color("#000000")
		});
	}

	static const Palette& Get()
	{
		return Instance().m_active;
	}
private:
	Palette m_active = DefaultPalette;
	PaletteManager() {}
	static PaletteManager& Instance()
	{
		static PaletteManager i;
		return i;
	}
};

struct RectangleRenderer
{
	tako::Vector2 size;
	tako::Color color;
};

struct SpriteRenderer
{
	tako::Vector2 size;
	tako::Texture* sprite;
	tako::Vector2 offset;
};

struct Camera
{
};

struct PaletteSprite
{
	tako::Bitmap source;
	tako::Texture texture;

	PaletteSprite(): source(1,1){}

	PaletteSprite(tako::Bitmap&& source, tako::Texture texture): source(std::move(source)), texture(texture) {}
};
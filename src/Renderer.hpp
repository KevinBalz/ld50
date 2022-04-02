#pragma once
#include "Tako.hpp"

using Palette = std::array<tako::Color, 4>;

constexpr Palette DefaultPalette = {tako::Color("#081820"), tako::Color("#346856"), tako::Color("#88c070"), tako::Color("#e0f8d0")};

struct RectangleRenderer
{
	tako::Vector2 size;
	tako::Color color;
};

struct SpriteRenderer
{
	tako::Vector2 size;
	tako::Sprite* sprite;
	tako::Vector2 offset;
};

struct PaletteSprite
{
	tako::Bitmap source;
	tako::Texture texture;

	PaletteSprite(): source(1,1){}

	PaletteSprite(tako::Bitmap&& source, tako::Texture texture): source(std::move(source)), texture(texture) {}
};
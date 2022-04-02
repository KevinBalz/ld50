#pragma once
#include "Math.hpp"

struct TilePosition
{
	int x;
	int y;
};

struct GridObject
{
	float x;
	float y;

	GridObject(int tileX, int tileY)
	{
		x = tileX;
		y = tileY;
	}

};
constexpr float EPSILON = 0.05;
struct MovingObject
{
	int targetX;
	int targetY;
	float gridSpeed;

	bool IsMoving(const GridObject& grid)
	{
		if (tako::mathf::abs(targetX - grid.x) > EPSILON)
		{
			return true;
		}

		if (tako::mathf::abs(targetY - grid.y) > EPSILON)
		{
			return true;
		}
		return false;
	}
};

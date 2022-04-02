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

namespace Grid
{
	bool Move(tako::Entity entity, int xDelta, int yDelta, tako::World& world)
	{
		auto& mov = world.GetComponent<MovingObject>(entity);
		auto& gridobj = world.GetComponent<GridObject>(entity);
		auto newX = mov.targetX + xDelta;
		auto newY = mov.targetY + yDelta;

		bool col = false;
		world.IterateComps<tako::Entity, MovingObject>([&](tako::Entity ent, MovingObject& m)
		{
			if (ent == entity)
			{
				return;
			}
			if (m.targetX == newX && m.targetY == newY)
			{
				col = true;
			}
		});
		if (col)
		{
			return true;
		}

		world.IterateComps<tako::Entity, GridObject>([&](tako::Entity ent, GridObject& g)
		{
			if (ent == entity)
			{
				return;
			}
			int xF = std::floor(g.x);
			int xC = std::ceil(g.x);
			int yF = std::floor(g.y);
			int yC = std::ceil(g.y);
			if ((xF == newX || xC == newX) && (yF == newY || yC == newY))
			{
				col = true;
			}
		});
		if (col)
		{
			return true;
		}

		mov.targetX = newX;
		mov.targetY = newY;

		return false;
	}
}
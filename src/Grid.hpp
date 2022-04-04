#pragma once
#include "Math.hpp"
#include "Facing.hpp"
#include <deque>

struct Tile
{
	int x;
	int y;

	bool IsNeighbour(const Tile& other)
	{
		return (std::abs(x - other.x) + std::abs(y - other.y)) <= 1;
	}

	friend Tile operator+(const Tile& a, const FaceDelta& b)
	{
		return { a.x + b.x, a.y + b.y };
	}

	friend bool operator==(const Tile& a, const Tile& b)
	{
		return a.x == b.x && a.y == b.y;
	}

	friend bool operator!=(const Tile& a, const Tile& b)
	{
		return !(a == b);
	}
};

template<>
struct std::hash<Tile>
{
	std::size_t operator()(Tile const& s) const noexcept
	{
		std::size_t h1 = std::hash<int>{}(s.x);
		std::size_t h2 = std::hash<int>{}(s.y);
		return h1 ^ (h2 << 1);
	}
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

	Tile GetTile()
	{
		return { (int) std::round(x), (int) std::round(y) };
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
	bool IsTileEmpty(Tile target, tako::World& world, std::optional<tako::Entity> entity)
	{
		bool col = false;
		world.IterateComps<Tile>([&](Tile& tile)
		{
			if (tile.x == target.x && tile.y == target.y)
			{
				col = true;
			}
		});
		if (col)
		{
			return false;
		}

		world.IterateComps<tako::Entity, MovingObject>([&](tako::Entity ent, MovingObject& m)
		{
			if (ent == entity)
			{
				return;
			}
			if (m.targetX == target.x && m.targetY == target.y)
			{
				col = true;
			}
		});
		if (col)
		{
			return false;
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
			if ((xF == target.x || xC == target.x) && (yF == target.y || yC == target.y))
			{
				col = true;
			}
		});
		if (col)
		{
			return false;
		}
		return true;
	}

	bool Move(tako::Entity entity, int xDelta, int yDelta, tako::World& world)
	{
		auto& mov = world.GetComponent<MovingObject>(entity);
		auto& gridobj = world.GetComponent<GridObject>(entity);
		auto newX = mov.targetX + xDelta;
		auto newY = mov.targetY + yDelta;

		bool col = false;
		world.IterateComps<Tile>([&](Tile& tile)
		{
			if (tile.x == newX && tile.y == newY)
			{
				col = true;
			}
		});
		if (col)
		{
			return true;
		}

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

	namespace
	{
		struct TilePath
		{
			Tile to;
			Tile from;
		};
	}

	std::optional<Tile> GetNextTileToTarget(tako::Entity entity, Tile target, tako::World& world)
	{
		auto& mov = world.GetComponent<MovingObject>(entity);
		Tile start = { mov.targetX , mov.targetY};
		std::deque<TilePath> queue;
		std::unordered_map<Tile, Tile> visited;
		std::unordered_map<Tile, bool> colMap;
		queue.push_back({start, start});

		while (!queue.empty())
		{
			TilePath node = queue.front();
			queue.pop_front();
			if (visited.count(node.to))
			{
				continue;
			}
			visited[node.to] = node.from;
			if (node.to == target)
			{
				break;
			}
			Tile neighbours[] =
			{
				{node.to.x - 1, node.to.y},
				{node.to.x + 1, node.to.y},
				{node.to.x, node.to.y - 1},
				{node.to.x, node.to.y + 1}
			};

			for (auto tile : neighbours)
			{
				if (!colMap.count(tile))
				{
					colMap[tile] = !IsTileEmpty(tile, world, entity);
				}
				if (colMap[tile] && tile != target) continue;
				queue.push_back({tile, node.to});
			}
		}

		if (!visited.count(target))
		{
			return {};
		}

		Tile next = target;
		while (visited[next] != start)
		{
			next = visited[next];
		}

		return next;
	}
}
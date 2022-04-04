#pragma once
#include "Facing.hpp"
#include "Inventory.hpp"
#include <deque>

struct Player
{
	bool human;
	FaceDirection facing;
	std::deque<InventoryItem> items;
	bool inventoryOpen = false;
	int inventorySlotSelection;

	bool RemoveHeldItem(InventoryItem item)
	{
		if (items.size() <= 0 || items[0] != item)
		{
			return false;
		}
		items.erase(items.begin());
		return true;
	}
};
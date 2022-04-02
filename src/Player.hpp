#pragma once
#include "Facing.hpp"
#include "Inventory.hpp"
#include <vector>

struct Player
{
	FaceDirection facing;
	bool human = false;
	std::vector<InventoryItem> items;
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
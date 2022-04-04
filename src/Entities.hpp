#pragma once
#include "World.hpp"
#include "Inventory.hpp"

struct Dog
{
	std::optional<InventoryItem> heldItem;
};

using InteractionCallback = std::function<bool(tako::World&, tako::Entity, tako::Entity)>;

struct Interactable
{
	InteractionCallback* callback;
};

struct Appliable
{
	std::function<bool(tako::World&, tako::Entity, tako::Entity, InventoryItem item)> callback;
};

struct ItemSpawner
{
	int x;
	int y;
	InventoryItem item;
	int day = -1;
};

struct ThrownItem
{
	InventoryItem item;
};

struct Door {};
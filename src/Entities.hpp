#pragma once
#include "World.hpp"
#include "Inventory.hpp"

struct Dog
{
};

struct Interactable
{
	std::function<bool(tako::World&, tako::Entity, tako::Entity)> callback;
};

struct Appliable
{
	std::function<bool(tako::World&, tako::Entity, tako::Entity, InventoryItem item)> callback;
};
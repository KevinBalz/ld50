#pragma once

enum class InventoryItem
{
	Bone,
	Stick,
	Medicine,
	DogFood,
	Ball,
	Snack,
	COUNT
};

struct Pickup
{
	InventoryItem item;
};

InventoryItem GetItemFromString(std::string_view str)
{
	if (str == "Bone")
	{
		return InventoryItem::Bone;
	}
	else if (str == "Stick")
	{
		return InventoryItem::Stick;
	}
	else if (str == "Medicine")
	{
		return InventoryItem::Medicine;
	}
	else if (str == "DogFood")
	{
		return InventoryItem::DogFood;
	}
	else if (str == "Ball")
	{
		return InventoryItem::Ball;
	}
	else if (str == "Snack")
	{
		return InventoryItem::Snack;
	}

	return InventoryItem::COUNT;
}

std::string GetItemToString(InventoryItem item)
{
	switch (item)
	{
		case InventoryItem::Bone: return "Bone";
		case InventoryItem::Stick: return "Stick";
		case InventoryItem::Medicine: return "Medicine";
		case InventoryItem::DogFood: return "DogFood";
		case InventoryItem::Ball: return "Ball";
		case InventoryItem::Snack: return "Snack";
		default: return "";
	}
}

std::string GetItemName(InventoryItem item)
{
	switch (item)
	{
		case InventoryItem::DogFood: return "Dog Food";
		case InventoryItem::Snack: return "Dog Treat";
		default: return GetItemToString(item);
	}
}

std::string GetItemPic(InventoryItem item)
{
	return "/" + GetItemToString(item) + ".png";
}
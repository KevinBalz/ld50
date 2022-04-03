#pragma once

enum class InventoryItem
{
	Bone,
	Stick,
	Medicine,
	DogFood,
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
		default: return "";
	}
}

std::string GetItemName(InventoryItem item)
{
	switch (item)
	{
		case InventoryItem::DogFood: return "Dog Food";
		default: return GetItemToString(item);
	}
}

std::string GetItemPic(InventoryItem item)
{
	return "/" + GetItemToString(item) + ".png";
}
#pragma once

enum class InventoryItem
{
	Bone,
	Stick,
	Medicine,
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

	return InventoryItem::COUNT;
}

std::string GetItemPic(InventoryItem item)
{
	switch (item)
	{
		case InventoryItem::Bone: return "/Bone.png";
		case InventoryItem::Stick: return "/Stick.png";
		case InventoryItem::Medicine: return "/Medicine.png";
		default: return "";
	}
}
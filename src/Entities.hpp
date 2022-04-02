#pragma once
#include "World.hpp"

struct Dog
{
};

struct Interactable
{
	std::function<bool(const tako::World&, tako::Entity, tako::Entity)> callback;
};
#include "DialogSystem.hpp"

DialogSystem& DialogSystem::Instance()
{
	static DialogSystem i;
	return i;
}

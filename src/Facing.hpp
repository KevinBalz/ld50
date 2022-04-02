#pragma once

enum class FaceDirection
{
	Up,
	Down,
	Left,
	Right
};

struct FaceDelta
{
	int x;
	int y;
};

FaceDelta GetFaceDelta(FaceDirection direction)
{
	switch (direction)
	{
		case FaceDirection::Up: return {0, 1};
		case FaceDirection::Down: return {0, -1};
		case FaceDirection::Left: return {-1, 0};
		case FaceDirection::Right: return {1, 0};
	}
}

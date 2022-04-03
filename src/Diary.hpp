#pragma once
#include "Routine.hpp"

struct DayStatus
{
	bool gaveMedicine;
	bool gaveFood;
};

class Diary
{
public:
	static void Init()
	{
		auto& s = Instance();
	}

	static void Reset()
	{
		auto& s = Instance();
		s.dayNumber = 1;
		s.currentDay = { false, false };
	}

	static void StartNextDayRoutine(std::optional<std::function<void()>> finishCallback)
	{
		static int i;
		static float count;
		i = 0;
		count = 0.3f;
		PaletteManager::Random();
		Routine::Register([=](auto input, float dt)
		{
			auto& s = Instance();
			count -= dt;
			auto cycles = s.dayNumber;
			if (count < 0)
			{
				count = 0.3f;
				if (i < cycles)
				{
					i++;
					if (i == cycles)
					{
						PaletteManager::Set({tako::Color("#000000"),tako::Color("#000000"),tako::Color("#000000"),tako::Color("#000000")});
						count = 2;
					}
					else
					{
						PaletteManager::Random();
					}

				}
				else
				{
					PaletteManager::Reset();
					s.StartNextDay();
					if (finishCallback)
					{
						finishCallback.value()();
					}
					return false;
				}
			}

			return true;
		});
	}

	static bool IsDailyTasksDone()
	{
		auto& s = Instance();
		return s.currentDay.gaveFood && s.currentDay.gaveMedicine;
	}

	static const DayStatus& GetDayStatus()
	{
		return Instance().currentDay;
	}

	static void GaveMedicine()
	{
		Instance().currentDay.gaveMedicine = true;
	}

	static void GaveFood()
	{
		Instance().currentDay.gaveFood= true;
	}

private:
	int dayNumber = 0;
	DayStatus currentDay;

	void StartNextDay()
	{
		if (!IsDailyTasksDone())
		{
			return;
		}
		dayNumber++;
		currentDay = { false, false };
	}

	Diary() {}
	static Diary& Instance()
	{
		static Diary i;
		return i;
	}
};
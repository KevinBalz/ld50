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
		s.dayNumber = 0;
		s.currentDay = { false, false };
	}

	static void StartNextDayRoutine(std::optional<std::function<void()>> finishCallback)
	{
		static int cycle;
		static float count;
		cycle = 0;
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
				if (cycle < cycles)
				{
					cycle++;
					if (cycle == cycles)
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
					DialogSystem::NightTimeReading(GetNightTimeReading(),[=]() mutable
					{
						PaletteManager::Reset();
						s.StartNextDay();
						if (finishCallback)
						{
							finishCallback.value()();
						}
					});
					return false;
				}
			}

			return true;
		});
	}

	static void StartNextDay()
	{
		auto& s = Instance();
		s.dayNumber++;
		s.currentDay = { false, false };
	}

	static const std::vector<const char*>& GetDailyAgenda()
	{
		auto& s = Instance();
		return s.dailyAgendas[s.dayNumber - 1];
	}

	static const std::vector<const char*>& GetNightTimeReading()
	{
		auto& s = Instance();
		return s.nightThoughts[s.dayNumber];
	}

	static bool IsDailyTasksDone()
	{
		auto& s = Instance();
		return s.currentDay.gaveFood && s.currentDay.gaveMedicine;
	}

	static int GetDay()
	{
		return Instance().dayNumber;
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

	const std::array<std::vector<const char*>, 7> dailyAgendas =
	{
		std::vector{"I got some medicine for Peanut"},
		std::vector{"Day 2"},
		std::vector{"Day 3"},
		std::vector{"Day 4"},
		std::vector{"Day 5"},
		std::vector{"Day 6"},
		std::vector{"Day 7"}
	};

	const std::array<std::vector<const char*>, 8> nightThoughts =
	{
		std::vector{"Day 1"},
		std::vector{"Give Treat", "Day 2"},
		std::vector{"Day 3"},
		std::vector{"Day 4"},
		std::vector{"Day 5"},
		std::vector{"Day 6"},
		std::vector{"Day 7"},
		std::vector{"Day 8"},
	};


	Diary() {}
	static Diary& Instance()
	{
		static Diary i;
		return i;
	}
};
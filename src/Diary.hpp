#pragma once
#include "Routine.hpp"

struct DayStatus
{
	bool gaveMedicine;
	bool gaveFood;
	bool activity;
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
		return s.currentDay.gaveFood && s.currentDay.gaveMedicine && s.currentDay.activity;
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

	static void DidActivity()
	{
		Instance().currentDay.activity = true;
	}

private:
	int dayNumber = 0;
	DayStatus currentDay;

	const std::array<std::vector<const char*>, 7> dailyAgendas =
	{
		std::vector{"I got some medicine for Peanut\nTo get him up to speed again", "And he needs his \nmunchies as well!"},
		std::vector{"Still not strong enough\nto go outside!", "But I got something for him"},
		std::vector{"It got better","Maybe it's ok to take a little\nwalk in the garden today"},
		std::vector{"That walk was more taxing\nthan I anticipated", "But I found something that\nthat will cheer him up!"},
		std::vector{"Taking a break was worth it","Let's play some ball today!"},
		std::vector{"Huh, where is Peanut", "I think he is up to something"},
		std::vector{"I'm not sure if it's a good idea.", "But let's go to the park anyway", "I think there won't be a better time"}
	};

	const std::array<std::vector<const char*>, 8> nightThoughts =
	{
		std::vector{"Day 1"},
		std::vector{"I hope we can go out soon", "Time outside with him was the best", "Day 2"},
		std::vector{"He really enjoyed his snack","I love seeing him so happy", "... Given the circumstances", "Day 3"},
		std::vector{"Going outside was great", "I hope there will be a opportunity,","to go to the park one more time again", "Peanut really loves the park", "Day 4"},
		std::vector{"Again not going to the park", "At least I could give him something", "But he deserves more", "Day 5"},
		std::vector{"We used to play ball for hours everyday", "I wish we played more while we could","Day 6"},
		std::vector{"Today he looked so alive", "He tries his best to cheer me up", "But it doesn't get better", "Day 7"},
		std::vector{"Day 8"},
	};


	Diary() {}
	static Diary& Instance()
	{
		static Diary i;
		return i;
	}
};
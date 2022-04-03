#pragma once
#include "Tako.hpp"
#include <vector>
#include <deque>

using RoutineFunc = std::function<bool(tako::Input*, float)>;

class Routine
{
public:
	static void Init()
	{
		auto& s = Instance();
	}

	static void Register(RoutineFunc&& func)
	{
		Instance().m_queue.emplace_back(func);
	}

	static void Update(tako::Input* input, float dt)
	{
		auto& s = Instance();
		s.m_routines.erase(
			std::remove_if(s.m_routines.begin(), s.m_routines.end(), [&](auto& rout)
			{
				return !rout(input, dt);
			}),
			s.m_routines.end()
		);
		while (!s.m_queue.empty())
		{
			s.m_routines.emplace_back(std::move(s.m_queue.front()));
			s.m_queue.pop_front();
		}
	}
private:
	std::vector<RoutineFunc> m_routines;
	std::deque<RoutineFunc> m_queue;

	Routine() {}
	static Routine& Instance()
	{
		static Routine i;
		return i;
	}
};
#pragma once
#include <string>
#include <functional>

class SceneManager
{
public:
	static void Init(std::function<void()>&& startGame, std::function<void(std::string)>&& loadRoom)
	{
		I().m_startGame = std::move(startGame);
		I().m_loadRoom = std::move(loadRoom);
	}

	static void StartBeginning()
	{
		I().m_startGame();
	}

	static void LoadRoom(std::string levelName)
	{
		I().m_loadRoom(std::move(levelName));
	}
private:
	std::function<void()> m_startGame;
	std::function<void(std::string)> m_loadRoom;

	SceneManager() {}
	static SceneManager& I()
	{
		static SceneManager i;
		return i;
	}
};
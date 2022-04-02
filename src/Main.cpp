#include "Tako.hpp"
#include "Game.hpp"

void Setup(void* gameData, const tako::SetupData& setup)
{
	auto game = new (gameData) Game();
	game->Setup(setup);
}

void Update(void* gameData, tako::Input* input, float dt)
{
	auto game = reinterpret_cast<Game*>(gameData);
	game->Update(input, dt);
}

void Draw(void* gameData)
{
	auto game = reinterpret_cast<Game*>(gameData);
	game->Draw();
}

void tako::InitTakoConfig(GameConfig& config)
{
	config.Setup = Setup;
	config.Update = Update;
	config.Draw = Draw;
	config.graphicsAPI = tako::GraphicsAPI::OpenGL;
	config.gameDataSize = sizeof(Game);
}

#include "MazeGame.h"

int main()
{
	MazeGame::initEnv();
	MazeGame game;
	game.mTodo = MazeGame::NEW_GAME;
	game.mTimeoutTotal = 30;
	game.mainLoop();

	MazeGame::quit();
	return 0;
}
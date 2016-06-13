#pragma once
#include "Maze.h"

struct MenuNode;

struct MazeGame
{
	enum ToDo
	{
		PLAY, MENU, NEW_GAME, QUIT
	};
	enum 
	{
		WINDOW_WIDTH = 740,
		WINDOW_HEIGHT = 600
	};

	Maze   maze;				// 当前迷宫
	COORD  mLastPos;			// 保存暂停前老鼠的位置
	RECT  outputRect;			// 输出文字提示的位置
	long   mTimeoutTotal;		// 总超时时间
	long   mTimeoutRest;		// 剩余时间
	ToDo   mTodo;				// 下一步操作
	HANDLE hEvent;				// 控制计时线程的变量
	BOOL   bHideResume;
	static const Size SIZES[4];	// 地图大小预设数组
	static const TCHAR
		MENU_MAIN[8][5],
		MENU_SIZE[5][6];

	MazeGame();

	void mainLoop();

	void menu();

	void gameLoop();

	void timing();

	void newgame();

	void showShortestPath();

	void loadMap();

	void editMap();

	void saveMap();

	void changeMazeSize();

	void putHint(LPCTSTR text);

	void setMapSize(int index);

	void onplay();

	// 继续计时线程
	void resumeTimer()
	{
		::SetEvent(hEvent);
	}

	// 暂停计时线程
	void pauseTimer()
	{
		::ResetEvent(hEvent);
	}

	static int getKey();

	/**
	* 多线程启动函数
	*/
	static DWORD WINAPI timerLauncher(LPVOID thiz);

	static void initEnv();
	static void quit();
};
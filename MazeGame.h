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

	Maze   maze;				// ��ǰ�Թ�
	COORD  mLastPos;			// ������ͣǰ�����λ��
	RECT  outputRect;			// ���������ʾ��λ��
	long   mTimeoutTotal;		// �ܳ�ʱʱ��
	long   mTimeoutRest;		// ʣ��ʱ��
	ToDo   mTodo;				// ��һ������
	HANDLE hEvent;				// ���Ƽ�ʱ�̵߳ı���
	BOOL   bHideResume;
	static const Size SIZES[4];	// ��ͼ��СԤ������
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

	// ������ʱ�߳�
	void resumeTimer()
	{
		::SetEvent(hEvent);
	}

	// ��ͣ��ʱ�߳�
	void pauseTimer()
	{
		::ResetEvent(hEvent);
	}

	static int getKey();

	/**
	* ���߳���������
	*/
	static DWORD WINAPI timerLauncher(LPVOID thiz);

	static void initEnv();
	static void quit();
};
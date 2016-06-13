#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <io.h>
#include <stdio.h>
#include "MazeGame.h"
#include "Menu.h"
#include "LinkStack.h"

/**
 * 地图尺寸选项
 */
const Size MazeGame::SIZES[4] = 
{
	{ 19, 11 },
	{ 27, 19 },
	{ 31, 23 },
	{ 35, 27 }
};

const TCHAR MazeGame::MENU_MAIN[][5] = {
	_T("继续游戏"),
	_T("新游戏　"),
	_T("最短路径"),
	_T("读取地图"),
	_T("编辑地图"),
	_T("保存地图"),
	_T("切换尺寸"),
	_T("退出游戏")
};

const TCHAR MazeGame::MENU_SIZE[][6] = {
	_T("19*11"),
	_T("27*19"),
	_T("31*23"),
	_T("35*27"),
	_T("返回 ")
};

MazeGame::MazeGame()
{
	maze.map = NULL;
	setMapSize(3);

	mTimeoutRest = 0;
	hEvent       = ::CreateEvent(NULL, TRUE, TRUE, _T("Timer"));
}


/**
 * 游戏主循环
 * 在gameLoop()中开启和暂停计时线程
 */
void MazeGame::mainLoop()
{
	while (1)
	{
		switch (mTodo)
		{
		case PLAY:
			onplay();
			break;
		case MENU:
			menu();
			break;
		case NEW_GAME:
			newgame();
			break;
		case QUIT:
		default:
			return;
		}
	}
}

// 显示菜单（暂停）
void MazeGame::menu()
{
	while (1)
	{
		printMenu(MENU_MAIN, _T("主菜单"), bHideResume);
		switch (getch())
		{
		case VK_ESCAPE:
		case '1':
			if(!bHideResume)
			{
				mTodo = PLAY;
				return;
			}
			break;
		case '2':
			mTodo = NEW_GAME;
			return;
		case '3':
			if(!bHideResume)
				showShortestPath();
			break;
		case '4':
			loadMap();
			break;
		case '5':
			if (!bHideResume)
				editMap();
			break;
		case '6':
			saveMap();
			break;
		case '7':
			changeMazeSize();
			break;
		case '0':
			mTodo = QUIT;
			return;
		}
	}
}

// 游戏循环
void MazeGame::gameLoop()
{
	int keyCode;			//用来接收按键
	COORD pos = mLastPos;	//老鼠当前位置

	if (mTimeoutRest == 0)
	{
		// 创建计时器
		HANDLE hThread = CreateThread(NULL, 0, timerLauncher, this, 0, NULL);
		CloseHandle(hThread);
	}
	// 继续计时
	resumeTimer();

	// 绘制入口
	maze.drawEntrance();
	// 绘制出口
	maze.drawExit();

	bHideResume = FALSE;

	for (;;)
	{
		maze.drawMouse(pos.X, pos.Y); //画出玩家当前位置

		if (pos == maze.exit) //判断是否到达出口
		{
			pauseTimer();
			putHint(_T("到达终点，按任意键结束"));
			bHideResume = TRUE;
			mTodo = MENU;
			getch();
			break;
		}
		keyCode = getKey();
		if (mTimeoutRest == 0)
		{
			// 时间到了
			bHideResume = TRUE;
			mTodo = MENU;
			break;
		}

		switch (keyCode)
		{
		// 暂停
		case VK_ESCAPE:
			mTodo = MENU;
			// 记录当前状态
			mLastPos = pos;
			// 暂停计时
			pauseTimer();
			return;
		case VK_UP: // 向上走
			if (maze.isRoad(pos.Y - 1, pos.X))
			{
				maze.paint(pos);
				--pos.Y;
			}
			break;
		case VK_DOWN: // 向下走
			if (maze.isRoad(pos.Y + 1, pos.X))
			{
				maze.paint(pos);
				++pos.Y;
			}
			break;
		case VK_LEFT: // 向左走
			if (maze.isRoad(pos.Y, pos.X - 1))
			{
				maze.paint(pos);
				--pos.X;
			}
			break;
		case VK_RIGHT: // 向右走
			if (maze.isRoad(pos.Y, pos.X + 1))
			{
				maze.paint(pos);
				++pos.X;
			}
			break;
		}
	}
}

// 计时逻辑
void MazeGame::timing()
{
	mTimeoutRest = mTimeoutTotal;
	TCHAR buf[8];
	while (mTimeoutRest != 0)
	{
		::WaitForSingleObject(hEvent, INFINITE);
		_stprintf(buf, _T("剩余时间：%02ld"), mTimeoutRest);
		putHint(buf);
		Sleep(1000);
		--mTimeoutRest;
	}
	// 时间到了
	putHint(_T("时间到了，任意键打开菜单"));
}

// 新游戏
void MazeGame::newgame()
{
	mLastPos = maze.entrance; // 回到入口处
	maze.randomMap();
	// 之前的计时线程未返回
	if(mTimeoutRest)
		mTimeoutRest = mTimeoutTotal;
	onplay();
}

// 显示最短路径
void MazeGame::showShortestPath()
{
	cleardevice();
	maze.paint();
	// 绘制入口
	maze.drawEntrance();
	// 绘制出口
	maze.drawExit();
	putHint(_T("按Esc键返回"));

	enum DI
	{
		EAST = 1, SOUTH, WEST, NORTH
	};
	struct StackElem
	{
		COORD pos;
		int   di; // 下一位置的方向
	};
	LinkStack<StackElem> stack;
	LinkStack<StackElem> stackMin;
	StackElem e;
	int path_count = 0;

	//设定当前位置的初值为入口位置
	e.pos = maze.entrance;
	do 
	{
		//当前位置是否可通
		bool can_pass = false;
		if (e.pos == maze.exit) {
			//到达终点
			++path_count;
			if (path_count == 1 || (stack.length() < stackMin.length()))
				stackMin = stack;
		}
		else {
			can_pass = maze.isRoad(e.pos.Y, e.pos.X);
			maze.isRoad(e.pos.Y, e.pos.X);
			if (can_pass)
			{
				auto s = stack.Top();
				while (s)
				{
					if (e.pos == s->data.pos)
					{
						can_pass = false;
						break;
					}
					s = s->next;
				}
			}
		}
		//若当前位置可通
		if (can_pass) {
			e.di = EAST;
			//将当前位置插入栈顶
			stack.Push(e);
			//Sleep(100);
			//maze.drawChar(e.pos.X, e.pos.Y, _T('＊'));
			//切换当前位置的东邻方块为新的当前位置
			++e.pos.X;
		}
		else
		{
			//若栈不空且栈顶位置尚有其他方向未经探索，
			//则设定新的当前位置为沿顺时针方向旋转找到的栈顶位置的下一相邻块；
			if (!stack.isEmpty())
			{
				stack.Pop(e);
				//若栈不空但栈顶位置的四周均不可通，
				//则删去栈顶位置；
				while (e.di == NORTH && !stack.isEmpty())
				{
					//Sleep(50);
					maze.drawRoad(e.pos.X, e.pos.Y);
					//从路径中删去该通道块
					stack.Pop(e);
				}
				if (e.di < NORTH)
				{
					++e.di;
					stack.Push(e);
					switch (e.di)
					{
					case SOUTH:
						++e.pos.Y;
						break;
					case WEST:
						--e.pos.X;
						break;
					case NORTH:
						--e.pos.Y;
						break;
					}
				}
			}
			//  直至找到一个可通的相邻块出栈至栈空；
		}
	} while (!stack.isEmpty());

	if (path_count == 0)
	{
		putHint(_T("没有出口！"));
	}
	else
	{
		//绘制路径
		auto s = stackMin.Top()->next;
		while (s)
		{
			maze.drawChar(s->data.pos.X, s->data.pos.Y, _T('＊'));
			s = s->next;
		}
		TCHAR buf[32];
		_stprintf(buf, _T("共%d条路径，最短路径为长度为%d"), path_count, stackMin.length());
		putHint(buf);
	}

	while (1)
	{
		char ch = getch();
		if(ch == VK_ESCAPE)
			break;
	}
}

void MazeGame::editMap()
{
	cleardevice();
	maze.paint();
	putHint(_T("按Esc键返回"));

	MOUSEMSG msg;

	FlushMouseMsgBuffer();
	while (true)
	{
		while (MouseHit())			// 当有鼠标消息的时候执行
		{
			msg = GetMouseMsg();	// 获取鼠标消息

			if (msg.uMsg == WM_LBUTTONDOWN)		// 左键单击
			{
				// 屏幕坐标转换成迷宫坐标
				msg.x = (msg.x / Maze::WALL_WIDTH) - 1;
				msg.y = (msg.y / Maze::WALL_WIDTH) - 1;
				maze.reverseBlockAndPaint(msg.y, msg.x);
			}
		}
		if (kbhit() && getch() == VK_ESCAPE)
		{
			break;
		}
		Sleep(200);
	}
	FlushMouseMsgBuffer();
}

void MazeGame::loadMap()
{
	struct _tfinddata_t data;
	long hnd = _tfindfirst(_T("./*.map"), &data);    // 查找文件名与正则表达式chRE的匹配第一个文件
	if (hnd < 0)
	{
		putHint(_T("没有地图文件"));
		getch();
	}
	else
	{
		int nRet = (hnd < 0) ? -1 : 1;
		int count = 0;
		MenuNode *list, *pNode;
		LPTSTR buf;

		list = new MenuNode;
		pNode = list;
		pNode->name = _T("读取地图");
		pNode->next = NULL;
		// 文件名保存到链表
		while (nRet >= 0)
		{
			if (data.attrib == _A_ARCH)  // 如果是文件
			{
				buf = new TCHAR[_tcslen(data.name) + 1];
				_tcscpy(buf, data.name);
				pNode->next = new MenuNode;
				pNode = pNode->next;
				pNode->name = buf;
				pNode->next = NULL;
				++count;
			}
			nRet = _tfindnext(hnd, &data);
		}

		pNode->next = new MenuNode;
		pNode = pNode->next;
		pNode->name = _T("返回");
		pNode->next = NULL;

		// 显示文件列表
		while (1)
		{
			printMenu(list);
			char ch = getch();
			if(ch ==  VK_ESCAPE || ch == '0')
				break;
			else 
			{
				ch -= '0';
				if (ch > 0 && ch <= count)
				{
					MenuNode *pFile = list->next;
					for (int i = 1; i <= count; ++i)
					{
						if (i == ch)
						{
							if (maze.load(pFile->name))
							{
								putHint(_T("读取成功"));
								bHideResume = FALSE;
							}
							else
								putHint(_T("读取失败"));
							getch();
							break;
						}
						pFile = pFile->next;
					}
					if(pFile->next != NULL)
						break;
				}
			}
		}

		// 销毁链表
		list->name = NULL; // 不是动态分配的name去掉
		pNode->name = NULL;
		while (list)
		{
			pNode = list;
			list = list->next;
			delete pNode->name;
			delete pNode;
		}
	}
	_findclose(hnd);     // 关闭当前句柄
}

void MazeGame::saveMap()
{
	TCHAR path[16];
	InputBox(path, 10, _T("请输入要保存的文件名，不超过10个字符"), _T("保存到文件"));
	if (*path)
	{
		_tcscat(path, _T(".map"));
		if (maze.save(path))
			putHint(_T("保存成功"));
		else
			putHint(_T("保存失败"));
	} else 
		putHint(_T("路径不能为空"));
	getch();
}

// 改变地图大小
void MazeGame::changeMazeSize()
{
	while (1)
	{
		cleardevice();
		printMenu(MENU_SIZE, L"地图大小");
		char ch = getch();
		switch (ch)
		{
		case '1':
		case '2':
		case '3':
		case '4':
			bHideResume = TRUE;
			setMapSize(ch - '1');
			return;
		case VK_ESCAPE:
		case '0':
			return;
		}
	}
}

void MazeGame::putHint(LPCTSTR text)
{
	clearrectangle(outputRect.left, outputRect.top, outputRect.right, outputRect.bottom);
	drawtext(text, &outputRect, DT_CENTER);
}

/**
 * 设置地图大小
 */
void MazeGame::setMapSize(int index)
{
	maze.init(SIZES[index]);

	outputRect.left = Maze::WALL_WIDTH;
	outputRect.top = Maze::mappos(maze.height);
	outputRect.right = Maze::mappos(maze.width);
	outputRect.bottom = outputRect.top + Maze::WALL_WIDTH;
}

// 游戏准备
void MazeGame::onplay()
{
	cleardevice();
	maze.paint();
	gameLoop();
}

/**
 * 接收按键
 */
int MazeGame::getKey()
{
	char c;
	while (c = getch())
	{
		/**
		 * 当 getch() 返回 0 或 0xE0 时，就表示用户按了功能键
		 * 这时候需要再调用一次 getch()， 返回的值表示功能键
		 */
		if (c == (char)0xE0 || c == 0)
		{
			switch (getch())
			{
			case 72: return VK_UP;   //上
			case 80: return VK_DOWN; //下
			case 75: return VK_LEFT; //左
			case 77: return VK_RIGHT;//右
			}
		}
		else
			switch (c)
			{
			case VK_ESCAPE:
			case 'p':
				return VK_ESCAPE;
			}
	}
	return 0;
}

DWORD MazeGame::timerLauncher(LPVOID thiz)
{
	((MazeGame*)thiz)->timing();
	return 0;
}

void MazeGame::initEnv()
{
	initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
	SetWindowText(GetHWnd(), _T("迷宫游戏"));

	srand((unsigned)time(NULL));				//初始化随机种子
	LOGFONT font;
	gettextstyle(&font);						// 获取当前字体设置
	font.lfHeight = 25;							// 设置字体高度
	_tcscpy(font.lfFaceName, _T("微软雅黑"));	// 设置字体
	font.lfQuality = ANTIALIASED_QUALITY;		// 抗锯齿
	font.lfWeight = 560;
	settextstyle(&font);
	settextcolor(0);

	// 加载图标
	loadimage(Maze::wall, _T("icon_block.jpg"), Maze::WALL_WIDTH, Maze::WALL_WIDTH);
	loadimage(Maze::mouse, _T("icon_mouse.jpg"));

	setbkcolor(0xdddddd);
	setfillcolor(getbkcolor());
	cleardevice();
}

void MazeGame::quit()
{
	closegraph();
}

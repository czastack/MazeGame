#include <graphics.h>
#include <conio.h>
#include <time.h>
#include <io.h>
#include <stdio.h>
#include "MazeGame.h"
#include "Menu.h"
#include "LinkStack.h"

/**
 * ��ͼ�ߴ�ѡ��
 */
const Size MazeGame::SIZES[4] = 
{
	{ 19, 11 },
	{ 27, 19 },
	{ 31, 23 },
	{ 35, 27 }
};

const TCHAR MazeGame::MENU_MAIN[][5] = {
	_T("������Ϸ"),
	_T("����Ϸ��"),
	_T("���·��"),
	_T("��ȡ��ͼ"),
	_T("�༭��ͼ"),
	_T("�����ͼ"),
	_T("�л��ߴ�"),
	_T("�˳���Ϸ")
};

const TCHAR MazeGame::MENU_SIZE[][6] = {
	_T("19*11"),
	_T("27*19"),
	_T("31*23"),
	_T("35*27"),
	_T("���� ")
};

MazeGame::MazeGame()
{
	maze.map = NULL;
	setMapSize(3);

	mTimeoutRest = 0;
	hEvent       = ::CreateEvent(NULL, TRUE, TRUE, _T("Timer"));
}


/**
 * ��Ϸ��ѭ��
 * ��gameLoop()�п�������ͣ��ʱ�߳�
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

// ��ʾ�˵�����ͣ��
void MazeGame::menu()
{
	while (1)
	{
		printMenu(MENU_MAIN, _T("���˵�"), bHideResume);
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

// ��Ϸѭ��
void MazeGame::gameLoop()
{
	int keyCode;			//�������հ���
	COORD pos = mLastPos;	//����ǰλ��

	if (mTimeoutRest == 0)
	{
		// ������ʱ��
		HANDLE hThread = CreateThread(NULL, 0, timerLauncher, this, 0, NULL);
		CloseHandle(hThread);
	}
	// ������ʱ
	resumeTimer();

	// �������
	maze.drawEntrance();
	// ���Ƴ���
	maze.drawExit();

	bHideResume = FALSE;

	for (;;)
	{
		maze.drawMouse(pos.X, pos.Y); //������ҵ�ǰλ��

		if (pos == maze.exit) //�ж��Ƿ񵽴����
		{
			pauseTimer();
			putHint(_T("�����յ㣬�����������"));
			bHideResume = TRUE;
			mTodo = MENU;
			getch();
			break;
		}
		keyCode = getKey();
		if (mTimeoutRest == 0)
		{
			// ʱ�䵽��
			bHideResume = TRUE;
			mTodo = MENU;
			break;
		}

		switch (keyCode)
		{
		// ��ͣ
		case VK_ESCAPE:
			mTodo = MENU;
			// ��¼��ǰ״̬
			mLastPos = pos;
			// ��ͣ��ʱ
			pauseTimer();
			return;
		case VK_UP: // ������
			if (maze.isRoad(pos.Y - 1, pos.X))
			{
				maze.paint(pos);
				--pos.Y;
			}
			break;
		case VK_DOWN: // ������
			if (maze.isRoad(pos.Y + 1, pos.X))
			{
				maze.paint(pos);
				++pos.Y;
			}
			break;
		case VK_LEFT: // ������
			if (maze.isRoad(pos.Y, pos.X - 1))
			{
				maze.paint(pos);
				--pos.X;
			}
			break;
		case VK_RIGHT: // ������
			if (maze.isRoad(pos.Y, pos.X + 1))
			{
				maze.paint(pos);
				++pos.X;
			}
			break;
		}
	}
}

// ��ʱ�߼�
void MazeGame::timing()
{
	mTimeoutRest = mTimeoutTotal;
	TCHAR buf[8];
	while (mTimeoutRest != 0)
	{
		::WaitForSingleObject(hEvent, INFINITE);
		_stprintf(buf, _T("ʣ��ʱ�䣺%02ld"), mTimeoutRest);
		putHint(buf);
		Sleep(1000);
		--mTimeoutRest;
	}
	// ʱ�䵽��
	putHint(_T("ʱ�䵽�ˣ�������򿪲˵�"));
}

// ����Ϸ
void MazeGame::newgame()
{
	mLastPos = maze.entrance; // �ص���ڴ�
	maze.randomMap();
	// ֮ǰ�ļ�ʱ�߳�δ����
	if(mTimeoutRest)
		mTimeoutRest = mTimeoutTotal;
	onplay();
}

// ��ʾ���·��
void MazeGame::showShortestPath()
{
	cleardevice();
	maze.paint();
	// �������
	maze.drawEntrance();
	// ���Ƴ���
	maze.drawExit();
	putHint(_T("��Esc������"));

	enum DI
	{
		EAST = 1, SOUTH, WEST, NORTH
	};
	struct StackElem
	{
		COORD pos;
		int   di; // ��һλ�õķ���
	};
	LinkStack<StackElem> stack;
	LinkStack<StackElem> stackMin;
	StackElem e;
	int path_count = 0;

	//�趨��ǰλ�õĳ�ֵΪ���λ��
	e.pos = maze.entrance;
	do 
	{
		//��ǰλ���Ƿ��ͨ
		bool can_pass = false;
		if (e.pos == maze.exit) {
			//�����յ�
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
		//����ǰλ�ÿ�ͨ
		if (can_pass) {
			e.di = EAST;
			//����ǰλ�ò���ջ��
			stack.Push(e);
			//Sleep(100);
			//maze.drawChar(e.pos.X, e.pos.Y, _T('��'));
			//�л���ǰλ�õĶ��ڷ���Ϊ�µĵ�ǰλ��
			++e.pos.X;
		}
		else
		{
			//��ջ������ջ��λ��������������δ��̽����
			//���趨�µĵ�ǰλ��Ϊ��˳ʱ�뷽����ת�ҵ���ջ��λ�õ���һ���ڿ飻
			if (!stack.isEmpty())
			{
				stack.Pop(e);
				//��ջ���յ�ջ��λ�õ����ܾ�����ͨ��
				//��ɾȥջ��λ�ã�
				while (e.di == NORTH && !stack.isEmpty())
				{
					//Sleep(50);
					maze.drawRoad(e.pos.X, e.pos.Y);
					//��·����ɾȥ��ͨ����
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
			//  ֱ���ҵ�һ����ͨ�����ڿ��ջ��ջ�գ�
		}
	} while (!stack.isEmpty());

	if (path_count == 0)
	{
		putHint(_T("û�г��ڣ�"));
	}
	else
	{
		//����·��
		auto s = stackMin.Top()->next;
		while (s)
		{
			maze.drawChar(s->data.pos.X, s->data.pos.Y, _T('��'));
			s = s->next;
		}
		TCHAR buf[32];
		_stprintf(buf, _T("��%d��·�������·��Ϊ����Ϊ%d"), path_count, stackMin.length());
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
	putHint(_T("��Esc������"));

	MOUSEMSG msg;

	FlushMouseMsgBuffer();
	while (true)
	{
		while (MouseHit())			// ���������Ϣ��ʱ��ִ��
		{
			msg = GetMouseMsg();	// ��ȡ�����Ϣ

			if (msg.uMsg == WM_LBUTTONDOWN)		// �������
			{
				// ��Ļ����ת�����Թ�����
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
	long hnd = _tfindfirst(_T("./*.map"), &data);    // �����ļ�����������ʽchRE��ƥ���һ���ļ�
	if (hnd < 0)
	{
		putHint(_T("û�е�ͼ�ļ�"));
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
		pNode->name = _T("��ȡ��ͼ");
		pNode->next = NULL;
		// �ļ������浽����
		while (nRet >= 0)
		{
			if (data.attrib == _A_ARCH)  // ������ļ�
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
		pNode->name = _T("����");
		pNode->next = NULL;

		// ��ʾ�ļ��б�
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
								putHint(_T("��ȡ�ɹ�"));
								bHideResume = FALSE;
							}
							else
								putHint(_T("��ȡʧ��"));
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

		// ��������
		list->name = NULL; // ���Ƕ�̬�����nameȥ��
		pNode->name = NULL;
		while (list)
		{
			pNode = list;
			list = list->next;
			delete pNode->name;
			delete pNode;
		}
	}
	_findclose(hnd);     // �رյ�ǰ���
}

void MazeGame::saveMap()
{
	TCHAR path[16];
	InputBox(path, 10, _T("������Ҫ������ļ�����������10���ַ�"), _T("���浽�ļ�"));
	if (*path)
	{
		_tcscat(path, _T(".map"));
		if (maze.save(path))
			putHint(_T("����ɹ�"));
		else
			putHint(_T("����ʧ��"));
	} else 
		putHint(_T("·������Ϊ��"));
	getch();
}

// �ı��ͼ��С
void MazeGame::changeMazeSize()
{
	while (1)
	{
		cleardevice();
		printMenu(MENU_SIZE, L"��ͼ��С");
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
 * ���õ�ͼ��С
 */
void MazeGame::setMapSize(int index)
{
	maze.init(SIZES[index]);

	outputRect.left = Maze::WALL_WIDTH;
	outputRect.top = Maze::mappos(maze.height);
	outputRect.right = Maze::mappos(maze.width);
	outputRect.bottom = outputRect.top + Maze::WALL_WIDTH;
}

// ��Ϸ׼��
void MazeGame::onplay()
{
	cleardevice();
	maze.paint();
	gameLoop();
}

/**
 * ���հ���
 */
int MazeGame::getKey()
{
	char c;
	while (c = getch())
	{
		/**
		 * �� getch() ���� 0 �� 0xE0 ʱ���ͱ�ʾ�û����˹��ܼ�
		 * ��ʱ����Ҫ�ٵ���һ�� getch()�� ���ص�ֵ��ʾ���ܼ�
		 */
		if (c == (char)0xE0 || c == 0)
		{
			switch (getch())
			{
			case 72: return VK_UP;   //��
			case 80: return VK_DOWN; //��
			case 75: return VK_LEFT; //��
			case 77: return VK_RIGHT;//��
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
	SetWindowText(GetHWnd(), _T("�Թ���Ϸ"));

	srand((unsigned)time(NULL));				//��ʼ���������
	LOGFONT font;
	gettextstyle(&font);						// ��ȡ��ǰ��������
	font.lfHeight = 25;							// ��������߶�
	_tcscpy(font.lfFaceName, _T("΢���ź�"));	// ��������
	font.lfQuality = ANTIALIASED_QUALITY;		// �����
	font.lfWeight = 560;
	settextstyle(&font);
	settextcolor(0);

	// ����ͼ��
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

#include <graphics.h>
#include "Maze.h"
#include "control.h"
#include <stdio.h>

const char Maze::MAP_HEAD[] = "an_maze_map";

IMAGE* Maze::wall = new IMAGE;
IMAGE* Maze::mouse = new IMAGE;

void Maze::init(const Size & sz)
{
	height = sz.height;
	width = sz.width;
	destroyMap();
	createMap();
}

/**
* ��ʼ����ͼ
*/
void Maze::createMap()
{
	bits32Col = width >> 5;
	if (width & 0x1F)
		++bits32Col;

	// ��ʼ����ںͳ���
	entrance.X = (width - 1) >> 1;
	entrance.Y = (height - 1) >> 1;
	exit.X = width - 2;
	exit.Y = height - 2;

	// �����ڴ�
	mMapSize = height * bits32Col * sizeof(int);
	map = (int*)malloc(mMapSize);
}

/**
* ���������ͼ
*/
void Maze::randomMap()
{
	if (!map)
		return;

	ZeroMemory(map, mMapSize);
	DFCRandomMap(entrance.Y, entrance.X);
}

/**
* �õݹ�ʵ��������������������ɵ�ͼ
*/
void Maze::DFCRandomMap(int i, int j)
{
	char c[4][2] = { 0,1, 1,0, 0,-1, -1,0 }; //�ĸ�����
	int k, d, t;
	//���������
	for (k = 0; k < 4; k++)
	{
		d = rand() % 4;
		t = c[k][0]; c[k][0] = c[d][0]; c[d][0] = t;
		t = c[k][1]; c[k][1] = c[d][1]; c[d][1] = t;
	}
	setToRoad(i, j); // ȥ����ǰ���ǽ
	for (k = 0; k < 4; k++)
	{
		int nY = i + 2 * c[k][0]; // ��һ���ڽӵ���к�
		int nX = j + 2 * c[k][1]; // ��һ���ڽӵ���к�

		if (nY > 0 && nY < height && nX > 0 && nX < width && !isRoad(nY, nX))
		{
			setToRoad(i + c[k][0], j + c[k][1]); // ��ͨ��ǰ�����һ���ڽӵ�֮���ǽ
			DFCRandomMap(nY, nX);
		}
	}
}

/**
* �����Թ�
*/
void Maze::paint()
{
	if (!map)
		return;
	int i, j;
	TCHAR buf[4];
	RECT rc;

	// ����С����
	settextstyle(WALL_WIDTH, 6, nullptr);

	BeginBatchDraw();
	// �����б���
	rc.left = 0;
	rc.top = 0;
	rc.bottom = WALL_WIDTH;
	rc.right = WALL_WIDTH;

	for (j = 0; j < width; ++j)
	{
		rc.left += WALL_WIDTH;
		rc.right += WALL_WIDTH;
		_stprintf(buf, _T("%2d"), j);
		drawtext(buf, &rc, DT_CENTER);
	}

	rc.left = 0;
	rc.right = WALL_WIDTH;
	for (i = 0; i < height; ++i)
	{
		// �����б���
		rc.top += WALL_WIDTH;
		rc.bottom += WALL_WIDTH;
		_stprintf(buf, _T("%2d"), i);
		drawtext(buf, &rc, DT_CENTER);

		for (j = 0; j < width; ++j)
		{
			if (isRoad(i, j))
				drawRoad(j ,i);
			else
				drawWall(j, i);;
		}
	}
	EndBatchDraw();
	// �ָ��ɴ�����
	settextstyle(0, 9, nullptr);
}

/**
* ��Ϸ�����л��Ƶ�����
*/
void Maze::paint(COORD & pos)
{
	if (pos == entrance)
		drawEntrance();
	else
		drawRoad(pos.X, pos.Y);
}

void Maze::drawMouse(int x, int y)
{
	putimage(mappos(x), mappos(y), mouse);
}

void Maze::drawWall(int x, int y)
{
	putimage(mappos(x), mappos(y), wall);
}

void Maze::drawRoad(int x, int y)
{
	x = mappos(x);
	y = mappos(y);
	clearrectangle(x, y, x + WALL_WIDTH, y + WALL_WIDTH);
}

void Maze::drawChar(int x, int y, TCHAR ch)
{
	RECT rc;
	// �������
	rc.left = mappos(x);
	rc.top = mappos(y);
	rc.right = rc.left + WALL_WIDTH;
	rc.bottom = rc.top + WALL_WIDTH;
	drawtext(ch, &rc, 0);
}

/**
 * ת��·��ǽ
 */
void Maze::reverseBlockAndPaint(int i, int j)
{
	int x = mappos(j);
	int y = mappos(i);
	if (isRoad(i, j))
	{
		setToWall(i, j);
		putimage(x, y, wall);
	}
	else
	{
		setToRoad(i, j);
		clearrectangle(x, y, x + WALL_WIDTH, y + WALL_WIDTH);
	}
}

bool Maze::load(LPCTSTR path)
{
	FILE *fp = _tfopen(path, L"rb");
	if (fp)
	{
		// ��֤�ļ�ͷ
		char header[sizeof(MAP_HEAD)];
		fread(header, sizeof(MAP_HEAD), 1, fp);
		if (strcmp(MAP_HEAD, header) == 0)
		{
			fread(&width, sizeof(width), 1, fp);
			fread(&height, sizeof(height), 1, fp);
			destroyMap();
			createMap();
			fread(map, mMapSize, 1, fp);
			return true;
		}
	}
	return false;
}

bool Maze::save(LPCTSTR path)
{
	FILE *fp = _tfopen(path, L"wb");
	if (fp)
	{
		fwrite(MAP_HEAD, sizeof(MAP_HEAD), 1, fp);
		fwrite(&width, sizeof(width), 1, fp);
		fwrite(&height, sizeof(height), 1, fp);
		fwrite(map, mMapSize, 1, fp);
		fclose(fp);
		return true;
	}
	return false;
}

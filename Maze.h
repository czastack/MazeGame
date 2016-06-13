#pragma once
#include <windows.h>

#define BitSet(n, i) ((n) | (1 << (i)))
#define BitClr(n, i) ((n) & ~(1 << (i)))
#define BitGet(n, i) (((n) >> (i)) & 1)

/**
* 比较坐标是否相等
*/
inline bool operator == (const COORD &p1, const COORD &p2)
{
	return p1.X == p2.X && p1.Y == p2.Y;
}

struct Size
{
	WORD width;
	WORD height;
};

class IMAGE;

/**
 * 迷宫
 * 入口是迷宫中央
 * 出口是右下角
 */
struct Maze {
	enum MyEnum
	{
		WALL_WIDTH = 20
	};

	WORD width;
	WORD height;
	WORD bits32Col;	// x方向上需要多少个32位
	int  *map;		// 地图数据
	size_t mMapSize;
	COORD entrance; // 入口
	COORD exit;		// 出口
	static const char MAP_HEAD[12];	// 地图文件的文件头
	static IMAGE *wall; // 墙壁图像
	static IMAGE *mouse;// 老鼠图像

	void init(const Size &sz);
	void createMap();
	void randomMap();
	void DFCRandomMap(int i, int j);

	/**
	 * 释放迷宫地图内存
	 */
	void destroyMap()
	{
		if (map) {
			free(map);
			map = NULL;
		}
	}

	/*!
	 映射坐标对应的32位整型
	 @param i 行号
	 @param j 列号
	 */
	int& getBits(int i, int j)
	{
		return map[i * bits32Col + (j >> 5)];
	}

	void setToRoad(int i, int j)
	{
		int &bits = getBits(i, j);
		bits = BitSet(bits, j & 0x1F);
	}

	void setToWall(int i, int j)
	{
		int &bits = getBits(i, j);
		bits = BitClr(bits, j & 0x1F);
	}

	bool isRoad(int i, int j)
	{
		return BitGet(getBits(i, j), j & 0x1F) == 1;
	}

	void paint();
	void paint(COORD &pos);
	void drawMouse(int x, int y);
	void drawWall(int x, int y);
	void drawRoad(int x, int y);
	void drawChar(int x, int y, TCHAR ch);
	void drawEntrance()
	{
		drawChar(entrance.X, entrance.Y, L'入');
	}
	void drawExit()
	{
		drawChar(exit.X, exit.Y, L'出');
	}
	void reverseBlockAndPaint(int i, int j);
	bool load(LPCTSTR path);
	bool save(LPCTSTR path);

	static int mappos(int n)
	{
		return (n + 1) * WALL_WIDTH;
	}
};
#pragma once
#include <windows.h>

#define BitSet(n, i) ((n) | (1 << (i)))
#define BitClr(n, i) ((n) & ~(1 << (i)))
#define BitGet(n, i) (((n) >> (i)) & 1)

/**
* �Ƚ������Ƿ����
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
 * �Թ�
 * ������Թ�����
 * ���������½�
 */
struct Maze {
	enum MyEnum
	{
		WALL_WIDTH = 20
	};

	WORD width;
	WORD height;
	WORD bits32Col;	// x��������Ҫ���ٸ�32λ
	int  *map;		// ��ͼ����
	size_t mMapSize;
	COORD entrance; // ���
	COORD exit;		// ����
	static const char MAP_HEAD[12];	// ��ͼ�ļ����ļ�ͷ
	static IMAGE *wall; // ǽ��ͼ��
	static IMAGE *mouse;// ����ͼ��

	void init(const Size &sz);
	void createMap();
	void randomMap();
	void DFCRandomMap(int i, int j);

	/**
	 * �ͷ��Թ���ͼ�ڴ�
	 */
	void destroyMap()
	{
		if (map) {
			free(map);
			map = NULL;
		}
	}

	/*!
	 ӳ�������Ӧ��32λ����
	 @param i �к�
	 @param j �к�
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
		drawChar(entrance.X, entrance.Y, L'��');
	}
	void drawExit()
	{
		drawChar(exit.X, exit.Y, L'��');
	}
	void reverseBlockAndPaint(int i, int j);
	bool load(LPCTSTR path);
	bool save(LPCTSTR path);

	static int mappos(int n)
	{
		return (n + 1) * WALL_WIDTH;
	}
};
#include "Menu.h"
#include <graphics.h>

/**
* 菜单显示
*/
void printMenu(MenuNode *list, int offset/* = 0*/)
{
	cleardevice();
	// 打印开头
	RECT rc;
	rc.left = 100;
	rc.top = 60;
	rc.right = getwidth() - rc.left;
	rc.bottom =  getheight() - rc.top;

	BeginBatchDraw();
	setlinecolor(BLUE);
	setlinestyle(0, 4);
	rectangle(rc.left, rc.top, rc.right, rc.bottom);
	TCHAR buf[16];

	settextstyle(36, 0, NULL);
	rc.left += 160;
	rc.right -= 160;

	// 打印标题
	rc.top -= 20;
	rc.bottom = rc.top + 40;
	drawtext(list->name, &rc, DT_CENTER);
	rc.top += 20;
	rc.bottom += 20;

	// 打印主体
	int i = 1 + offset;
	MenuNode *pNode = list->next;
	while (pNode) {
		rc.top += 40;
		rc.bottom += 40;
		// 返回选项以0为标号
		if (pNode->next == NULL)
			i = 0;
		_stprintf(buf, _T("%d. %s"), i, pNode->name);
		drawtext(buf, &rc, 0);
		pNode = pNode->next;
		++i;
	}
	settextstyle(20, 0, NULL);
	EndBatchDraw();
}


void printMenu(const TCHAR * items, int rows, int cols, LPCTSTR title/* = NULL*/, int offset/* = 0*/)
{
	MenuNode *list, *pNode;
	list = new MenuNode;
	pNode = list;
	pNode->name = title;
	pNode->next = NULL;
	for (int i = offset; i < rows; ++i)
	{
		pNode->next = new MenuNode;
		pNode = pNode->next;
		pNode->name = items + i * cols;
		pNode->next = NULL;
	}

	printMenu(list, offset);

	// 销毁链表
	while (list)
	{
		pNode = list;
		list = list->next;
		delete pNode;
	}
}

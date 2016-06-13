#pragma once
#include <wtypes.h>

// 带头节点的菜单链表
struct MenuNode
{
	LPCTSTR name;
	MenuNode *next;
};

void printMenu(const TCHAR * items, int rows, int cols, LPCTSTR title = NULL, int offset = 0);
void printMenu(MenuNode *list, int offset = 0);

template<size_t rows, size_t cols>
inline void printMenu(const TCHAR(&arr)[rows][cols], LPCTSTR title = NULL, int offset = 0)
{
	printMenu((LPCTSTR)arr, rows, cols, title, offset);
}
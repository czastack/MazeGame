#pragma once

template<class T>
class LinkStack
{
	struct Node
	{
		T data;
		Node *next;
	} *top;
public:
	//构造函数，置空链栈
	LinkStack() 
	{
		top = NULL;
	}

	// 析构函数，释放链栈中各结点的存储空间
	~LinkStack()
	{
		ClearStack();
	}

	Node* Top() const
	{
		return top;
	}

	// 元素x入栈
	void Push(T &e);

	// 栈顶元素出栈
	void Pop(T &e);

	// 取栈顶元素
	void GetTop(T &e);

	// 判断栈是否为空
	bool isEmpty()
	{
		return top == NULL;
	}

	//清空栈
	void ClearStack();

	//获取栈元素个数
	int length();

	LinkStack<T>& operator= (const LinkStack<T> &other);
};

template<class T>
void LinkStack<T>::Push(T &e)
{
	Node *s = new Node;
	s->data = e;
	s->next = top;
	top = s; //新结点链入表首，为栈顶
}

template<class T>
void LinkStack<T>::Pop(T &e)
{
	Node *s;
	if (top == NULL)
		return;
	e = top->data;
	s = top;
	top = top->next;	//栈顶后移
	delete s;			//删除原栈顶结点
}

template<class T>
void LinkStack<T>::GetTop(T &e)
{
	if (top == NULL)
		return;
	e = top->data;
}

template<class T>
void LinkStack<T>::ClearStack()
{
	// 从栈顶开始释放栈链的每一个结点的存储空间
	while (top)
	{
		Node *s = top;
		top = top->next;
		delete s;
	}
}

template<class T>
int LinkStack<T>::length()
{
	int len = 0;
	auto s = Top();
	while (s)
	{
		++len;
		s = s->next;
	}
	return len;
}

template<class T>
LinkStack<T> & LinkStack<T>::operator=(const LinkStack<T> & other)
{
	ClearStack();
	auto s = other.Top();
	while (s)
	{
		Push(s->data);
		s = s->next;
	}
	return *this;
}

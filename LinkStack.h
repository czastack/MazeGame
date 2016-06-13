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
	//���캯�����ÿ���ջ
	LinkStack() 
	{
		top = NULL;
	}

	// �����������ͷ���ջ�и����Ĵ洢�ռ�
	~LinkStack()
	{
		ClearStack();
	}

	Node* Top() const
	{
		return top;
	}

	// Ԫ��x��ջ
	void Push(T &e);

	// ջ��Ԫ�س�ջ
	void Pop(T &e);

	// ȡջ��Ԫ��
	void GetTop(T &e);

	// �ж�ջ�Ƿ�Ϊ��
	bool isEmpty()
	{
		return top == NULL;
	}

	//���ջ
	void ClearStack();

	//��ȡջԪ�ظ���
	int length();

	LinkStack<T>& operator= (const LinkStack<T> &other);
};

template<class T>
void LinkStack<T>::Push(T &e)
{
	Node *s = new Node;
	s->data = e;
	s->next = top;
	top = s; //�½��������ף�Ϊջ��
}

template<class T>
void LinkStack<T>::Pop(T &e)
{
	Node *s;
	if (top == NULL)
		return;
	e = top->data;
	s = top;
	top = top->next;	//ջ������
	delete s;			//ɾ��ԭջ�����
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
	// ��ջ����ʼ�ͷ�ջ����ÿһ�����Ĵ洢�ռ�
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

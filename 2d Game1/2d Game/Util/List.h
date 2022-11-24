#pragma once

template<typename T>
class List
{
public:
	struct Node
	{
		T		_data;
		Node* _prev;
		Node* _Next;
	};

	class iterator
	{
	public:
		Node* _node;
	public:
		iterator(Node* node = nullptr) : _node(node)
		{
		}

		const iterator operator ++(int)
		{
			Node* prevNode = _node;
			_node = _node->_Next;
			return prevNode;
		}

		iterator& operator ++()
		{
			_node = _node->_Next;
			return *this;
		}

		const iterator operator --(int)
		{
			Node* prevNode = _node;
			_node = _node->_prev;
			return prevNode;
		}

		iterator& operator --()
		{
			_node = _node->_prev;
			return *this;
		}

		T& operator *()
		{
			return _node->_data;
		}

		bool operator == (const iterator& other)
		{
			return (this->_node == other._node);
		}

		bool operator != (const iterator& other)
		{
			return (this->_node != other._node);
		}
	};

public:
	List() : _size(0)
	{
		_head._prev = nullptr;
		_head._Next = &_tail;
		_tail._prev = &_head;
		_tail._Next = nullptr;
	}
	~List()
	{
		clear();
	}

	iterator begin()
	{
		return _head._Next;
	}

	iterator end()
	{
		return &_tail;
	}

	bool front(T* outValue)
	{
		if (_head._Next == nullptr)
		{
			return false;
		}
		*outValue = _head._Next->_data;
		return true;
	}

	bool back(T* outValue)
	{
		if (_tail._Prev == nullptr)
		{
			return false;
		}

		*outValue = _tail._Prev->_data;
		return true;
	}

	void push_front(T data)
	{
		Node* node = new Node;
		node->_data = data;

		node->_prev = &_head;
		node->_Next = _head._Next;
		_head._Next->_prev = node;
		_head._Next = node;

		this->_size++;
	}

	void push_back(T data)
	{
		Node* node = new Node;
		node->_data = data;

		node->_prev = _tail._prev;
		node->_Next = &_tail;
		_tail._prev->_Next = node;
		_tail._prev = node;

		this->_size++;
	}

	bool pop_front()
	{
		Node* popNode = _head._Next;
		if (popNode == &_tail)
		{
			return false;
		}

		_head._Next = popNode->_Next;
		popNode->_Next->_prev = &_head;

		delete popNode;
		this->_size--;
		return true;
	}
	bool pop_back()
	{
		Node* popNode = _tail._prev;
		if (popNode == &_head)
		{
			return false;
		}

		_tail._prev = popNode->_prev;
		popNode->_prev->_Next = &_tail;

		delete popNode;
		this->_size--;
		return true;
	}

	void clear()
	{
		while (pop_front()) {}
	}

	int size() { return _size; }
	bool empty() { return _size == 0; }

	iterator erase(iterator iter)
	{
		Node* delNode = iter._node;
		delNode->_prev->_Next = delNode->_Next;
		delNode->_Next->_prev = delNode->_prev;

		iterator retIter(delNode->_Next);
		delete delNode;
		this->_size--;

		return retIter;
	}

private:
	int		_size = 0;
	Node	_head;
	Node	_tail;
};

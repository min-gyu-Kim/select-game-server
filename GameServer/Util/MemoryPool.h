#pragma once
#include <new.h>
#include "List.h"

static int GenerateUniqueID()
{
	static unsigned char sID = 0x00;
	unsigned int id = sID << 24 | sID << 16 | sID << 8 | sID;
	return id;
}

template <class DATA>
class MemoryPool 
{
private:

	/* **************************************************************** */
	// 각 블럭 앞에 사용될 노드 구조체.
	/* **************************************************************** */
	struct st_BLOCK_NODE
	{
		st_BLOCK_NODE()
		{
			stpNextBlock = NULL;
		}
		unsigned int guard1;
		DATA data;
		unsigned int guard2;
		st_BLOCK_NODE *stpNextBlock;
	};

private:
	void newNode(DATA data)
	{
		st_BLOCK_NODE* node = new st_BLOCK_NODE;
		node->guard1 = m_ID;
		node->data = data;
		node->stpNextBlock = _pFreeNode;
		node->guard2 = m_ID;
		_pFreeNode = node;
		m_iAllocCount++;

		m_allocLecord.push_back(node);
	}

	void newNode()
	{
		st_BLOCK_NODE* node = new st_BLOCK_NODE;
		node->guard1 = m_ID;
		node->stpNextBlock = _pFreeNode;
		node->guard2 = m_ID;
		_pFreeNode = node;

		m_allocLecord.push_back(node);
	}

public:

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Parameters:	(int) 초기 블럭 개수.
	//				(bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
	// Return:
	//////////////////////////////////////////////////////////////////////////
	MemoryPool(int iBlockNum, bool bPlacementNew = false)
	{
		m_iAllocCount = 0;
		m_iUseCount = 0;
		m_bPlacementNew = bPlacementNew;
		_pFreeNode = nullptr;
		m_ID = GenerateUniqueID();

		for (int i = 0; i < iBlockNum; i++)
		{
			newNode();
		}
	}
	virtual	~MemoryPool()
	{
		for (const auto& ptr : m_allocLecord)
		{
			delete ptr;
		}
		m_allocLecord.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	// 블럭 하나를 할당받는다.  
	//
	// Parameters: 없음.
	// Return: (DATA *) 데이타 블럭 포인터.
	//////////////////////////////////////////////////////////////////////////
	DATA* Alloc(void)
	{
		if (_pFreeNode == nullptr)
		{
			newNode();
		}
		DATA* data = &_pFreeNode->data;
		if (m_bPlacementNew)
		{
			new (data)DATA;
		}
		_pFreeNode = _pFreeNode->stpNextBlock;
		m_iUseCount++;
		return data;
	}

	//////////////////////////////////////////////////////////////////////////
	// 사용중이던 블럭을 해제한다.
	//
	// Parameters: (DATA *) 블럭 포인터.
	// Return: (BOOL) TRUE, FALSE.
	//////////////////////////////////////////////////////////////////////////
	bool	Free(DATA* pData)
	{
		st_BLOCK_NODE* node = (st_BLOCK_NODE*)((unsigned int*)pData - 1);
		if (node->guard1 != m_ID ||
			node->guard2 != m_ID)
		{
			return false;
		}

		node->stpNextBlock = _pFreeNode;
		_pFreeNode = node;
		m_iUseCount--;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
	//
	// Parameters: 없음.
	// Return: (int) 메모리 풀 내부 전체 개수
	//////////////////////////////////////////////////////////////////////////
	int		GetAllocCount(void) { return m_iAllocCount; }

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 블럭 개수를 얻는다.
	//
	// Parameters: 없음.
	// Return: (int) 사용중인 블럭 개수.
	//////////////////////////////////////////////////////////////////////////
	int		GetUseCount(void) { return m_iUseCount; }


	// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.
private:
	bool m_bPlacementNew;
	int m_iAllocCount;
	int m_iUseCount;
	st_BLOCK_NODE *_pFreeNode;
	List<void*> m_allocLecord;

	unsigned int m_ID;
};
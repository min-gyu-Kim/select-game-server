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
	// �� �� �տ� ���� ��� ����ü.
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
	// ������, �ı���.
	//
	// Parameters:	(int) �ʱ� �� ����.
	//				(bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
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
	// �� �ϳ��� �Ҵ�޴´�.  
	//
	// Parameters: ����.
	// Return: (DATA *) ����Ÿ �� ������.
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
	// ������̴� ���� �����Ѵ�.
	//
	// Parameters: (DATA *) �� ������.
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
	// ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
	//
	// Parameters: ����.
	// Return: (int) �޸� Ǯ ���� ��ü ����
	//////////////////////////////////////////////////////////////////////////
	int		GetAllocCount(void) { return m_iAllocCount; }

	//////////////////////////////////////////////////////////////////////////
	// ���� ������� �� ������ ��´�.
	//
	// Parameters: ����.
	// Return: (int) ������� �� ����.
	//////////////////////////////////////////////////////////////////////////
	int		GetUseCount(void) { return m_iUseCount; }


	// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.
private:
	bool m_bPlacementNew;
	int m_iAllocCount;
	int m_iUseCount;
	st_BLOCK_NODE *_pFreeNode;
	List<void*> m_allocLecord;

	unsigned int m_ID;
};
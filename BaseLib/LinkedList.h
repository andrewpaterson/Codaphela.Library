#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__
#include "Define.h"
#include "DataCallback.h"


struct SLLNode
{
	SLLNode*	psNext;
	SLLNode*	psPrev;
};


class CLinkedList
{
protected:
	SLLNode*	mpsHead;
	SLLNode*	mpsTail;

public:
	void		Init(void);
	void		Kill(void);

	void		InsertAfterTail(SLLNode* psNewNode);
	void		InsertBeforeHead(SLLNode* psNewNode);
	void		InsertBeforeNode(SLLNode* psExistingNode, SLLNode* psNewNode);
	void		InsertAfterNode(SLLNode* psExistingNode, SLLNode* psNewNode);

	SLLNode* 	GetHead(void);
	SLLNode* 	GetTail(void);
	SLLNode* 	GetNext(SLLNode* psExistingNode);
	SLLNode* 	GetPrev(SLLNode* psExistingNode);

	void		RemoveTail(void);
	void		Remove(SLLNode* psNode);
	BOOL		SafeRemove(SLLNode* psNode);

	void		MoveToHead(SLLNode* psNode);
	void		MoveToTTail(SLLNode* psNode);

	int			NumElements(void);

	SLLNode*	Get(int iNum);
	int			IndexOf(SLLNode* psExistingNode);
	BOOL		IsInList(SLLNode* pcNode);

	void		BubbleSort(DataCompare fCompare, size_t iOffset);
	void		Swap(SLLNode* psNode1, SLLNode* psNode2);
	void		InsertIntoSorted(DataCompare fCompare, SLLNode* psNode, size_t iOffset);
};


#endif // !__LINKED_LIST_H__


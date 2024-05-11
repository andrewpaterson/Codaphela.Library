#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__
#include "PrimitiveTypes.h"
#include "DataCompare.h"


struct SLLNode
{
	SLLNode*	psNext;
	SLLNode*	psPrev;
};


//This is not the class you're looking for.  You want CLinkedListBlock.
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

	void		RemoveHead(void);
	void		RemoveTail(void);
	void		Remove(SLLNode* psNode);
	bool		SafeRemove(SLLNode* psNode);

	void		MoveToHead(SLLNode* psNode);
	void		MoveToTTail(SLLNode* psNode);

	size		NumElements(void);

	SLLNode*	Get(size iNum);
	size		IndexOf(SLLNode* psExistingNode);
	bool		IsInList(SLLNode* pcNode);

	void		BubbleSort(DataCompare fCompare, size iOffset);
	void		Swap(SLLNode* psNode1, SLLNode* psNode2);
	void		InsertIntoSorted(DataCompare fCompare, SLLNode* psNode, size iOffset);
};


#endif // __LINKED_LIST_H__


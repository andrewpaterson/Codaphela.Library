#include "PointerRemapper.h"
#include "LinkedList.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::Init(void)
{
	mpsHead = NULL;
	mpsTail = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::Kill(void)
{
	mpsHead = NULL;
	mpsTail = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::InsertAfterTail(SLLNode* psNode)
{
	if (psNode)
	{
		psNode->psNext = NULL;
		psNode->psPrev = mpsTail;

		if (mpsHead == NULL)
		{
			mpsHead = psNode;
		}
		else
		{
			mpsTail->psNext = psNode;
		}
		mpsTail = psNode;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::InsertBeforeHead(SLLNode* psNode)
{
	if (psNode)
	{
		psNode->psNext = mpsHead;
		psNode->psPrev = NULL;

		if (mpsTail == NULL)
		{
			mpsTail = psNode;
		}
		else
		{
			mpsHead->psPrev = psNode;
		}
		mpsHead = psNode;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::InsertBeforeNode(SLLNode* psExistingNode, SLLNode* psNewNode)
{
	SLLNode* psPos;
	SLLNode* psNode;

	if ((psExistingNode == NULL) || (psNewNode == NULL))
	{
		return;
	}

	psNode = psNewNode;
	psPos = psExistingNode;

	psNode->psPrev = psPos->psPrev;
	psNode->psNext = psPos;

	if (psPos->psPrev)
	{
		psPos->psPrev->psNext = psNode;
	}
	else
	{
		mpsHead = psNode;
	}
	psPos->psPrev = psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::InsertAfterNode(SLLNode* psExistingNode, SLLNode* psNewNode)
{
	SLLNode* psPos;
	SLLNode* psNode;

	if ((psExistingNode == NULL) || (psNewNode == NULL))
	{
		return;
	}

	psNode = psNewNode;
	psPos = psExistingNode;

	psNode->psPrev = psPos;
	psNode->psNext = psPos->psNext;

	if (psPos->psNext)
	{
		psPos->psNext->psPrev = psNode;
	}
	else
	{
		mpsTail = psNode;
	}
	psPos->psNext = psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SLLNode* CLinkedList::GetHead(void)
{
	return mpsHead;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SLLNode* CLinkedList::GetTail(void)
{
	return mpsTail;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SLLNode* CLinkedList::GetNext(SLLNode* psExistingNode)
{
	return psExistingNode->psNext;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SLLNode* CLinkedList::GetPrev(SLLNode* psExistingNode)
{
	return psExistingNode->psPrev;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::RemoveTail(void)
{

}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::Remove(SLLNode* psNode)
{
	if (psNode)
	{
		if (psNode->psPrev)
		{
			psNode->psPrev->psNext = psNode->psNext;
		}
		else
		{
			mpsHead = psNode->psNext;
		}

		if (psNode->psNext)
		{
			psNode->psNext->psPrev = psNode->psPrev;
		}
		else
		{
			mpsTail = psNode->psPrev;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CLinkedList::SafeRemove(SLLNode* psNode)
{
	if (IsInList(psNode))
	{
		Remove(psNode);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::MoveToHead(SLLNode* psNode)
{
	Remove(psNode);
	InsertBeforeHead(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::MoveToTTail(SLLNode* psNode)
{
	Remove(psNode);
	InsertAfterTail(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkedList::NumElements(void)
{
	int			iCount;
	SLLNode*	psNode;

	iCount = 0;
	psNode = GetHead();
	while (psNode)
	{
		iCount++;
		psNode = GetNext(psNode);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SLLNode* CLinkedList::Get(int iNum)
{
	int			iCount;
	SLLNode*	psData;

	psData = GetHead();
	for (iCount = 0;; iCount++)
	{
		if (psData)
		{
			if (iCount == iNum)
			{
				return psData;
			}
			psData = GetNext(psData);
		}
		else
		{
			return NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkedList::IndexOf(SLLNode* psNode)
{
	int			iIndex;
	SLLNode*	psCurrent;

	iIndex = 0;

	psCurrent = GetHead();
	while (psCurrent)
	{
		if (psCurrent == psNode)
		{
			return iIndex;
		}
		iIndex++;
		psCurrent = GetNext(psCurrent);
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CLinkedList::IsInList(SLLNode* psNode)
{
	int iIndex;

	iIndex = IndexOf(psNode);
	return iIndex != -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::BubbleSort(DataCompare fCompare, size_t iOffset)
{
	SLLNode*	psCurrent;
	SLLNode*	psNext;
	int			iResult;
	bool		bSwapped;
	void*		pvCurrent;
	void*		pvNext;


	psCurrent = GetHead();
	if (!psCurrent)
	{
		return;
	}

	bSwapped = true;
	while (bSwapped)
	{
		psCurrent = GetHead();
		psNext = GetNext(psCurrent);
		bSwapped = false;

		while ((psNext) && (psCurrent))
		{
			pvCurrent = RemapSinglePointer(psCurrent, iOffset);
			pvNext = RemapSinglePointer(psNext, iOffset);
			iResult = fCompare(pvCurrent, pvNext);
			if (iResult > 0)
			{
				Swap(psCurrent, psNext);
				bSwapped = true;
			}
			else
			{
				psCurrent = GetNext(psCurrent);
			}
			if (psCurrent)
			{
				psNext = GetNext(psCurrent);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::Swap(SLLNode* psNode1, SLLNode* psNode2)
{
	SLLNode		sNodeTemp1;
	SLLNode		sNodeTemp2;

	//Swapping a node for NULL is stupid
	if ((psNode1 == NULL) || (psNode2 == NULL) || (psNode1 == psNode2))
	{
		return;
	}

	sNodeTemp1.psNext = psNode1->psNext;
	sNodeTemp1.psPrev = psNode1->psPrev;
	sNodeTemp2.psNext = psNode2->psNext;
	sNodeTemp2.psPrev = psNode2->psPrev;

	if ((psNode1->psNext != psNode2) && (psNode1->psPrev != psNode2))
	{
		//Nodes to be swapped do not point at each other
		if (sNodeTemp1.psPrev)	{ sNodeTemp1.psPrev->psNext = psNode2; } else { mpsHead = psNode2; }
		if (sNodeTemp1.psNext)	{ sNodeTemp1.psNext->psPrev = psNode2; } else { mpsTail = psNode2; }
		if (sNodeTemp2.psPrev)	{ sNodeTemp2.psPrev->psNext = psNode1; } else { mpsHead = psNode1; }
		if (sNodeTemp2.psNext)	{ sNodeTemp2.psNext->psPrev = psNode1; } else { mpsTail = psNode1; }

		psNode1->psNext = sNodeTemp2.psNext;
		psNode1->psPrev = sNodeTemp2.psPrev;
		psNode2->psNext = sNodeTemp1.psNext;
		psNode2->psPrev = sNodeTemp1.psPrev;
	} 
	else if (psNode1->psNext==psNode2)
	{
		if (sNodeTemp1.psPrev)	{ sNodeTemp1.psPrev->psNext=psNode2; } else { mpsHead=psNode2; }
		if (sNodeTemp2.psNext)	{ sNodeTemp2.psNext->psPrev=psNode1; } else { mpsTail=psNode1; }

		psNode1->psNext = sNodeTemp2.psNext;
		psNode1->psPrev = psNode2;
		psNode2->psNext = psNode1;
		psNode2->psPrev = sNodeTemp1.psPrev;
	}
	else if (psNode1->psPrev==psNode2)
	{
		if (sNodeTemp1.psNext)	{ sNodeTemp1.psNext->psPrev=psNode2; } else { mpsTail=psNode2; }
		if (sNodeTemp2.psPrev)	{ sNodeTemp2.psPrev->psNext=psNode1; } else { mpsHead=psNode1; }

		psNode1->psNext = psNode2;
		psNode1->psPrev = sNodeTemp2.psPrev;
		psNode2->psNext = sNodeTemp1.psNext;
		psNode2->psPrev = psNode1;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedList::InsertIntoSorted(DataCompare fCompare, SLLNode* psNode, size_t iOffset)
{
	SLLNode*	psCurrent;
	int			iResult;
	void*		pvCurrent;
	void*		pvNext;

	psCurrent = GetHead();

	for (;;)
	{
		pvCurrent = RemapSinglePointer(psCurrent, iOffset);
		pvNext = RemapSinglePointer(psNode, iOffset);
		iResult = fCompare(pvNext, pvCurrent);
		if (iResult < 0)
		{
			InsertBeforeNode(psCurrent, psNode);
			break;
		}
		psCurrent = GetNext(psCurrent);
		if (psCurrent == NULL)
		{
			InsertAfterTail(psNode);
			break;
		}
	}
}


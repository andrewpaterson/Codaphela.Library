/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __TEMPLATE_LIST_H__
#define __TEMPLATE_LIST_H__
#include "Define.h"
#include "DataMacro.h"


//For the LinkedList.
struct SDNode
{
	SDNode*		psNext;
	SDNode*		psPrev;
};


template<class M>
class __CLinkListTemplate
{
public:
	SDNode*		mpsHead; 
	SDNode*		mpsTail;
	int			miNumElements;

	void	Kill(void);
	M* 		GetHead(void);
	M* 		GetTail(void);
	M* 		GetNext(M* pvData);
	M* 		GetPrev(M* pvData);
	M*		Get(int iNum);
	void	InsertDetachedAfterTail(M* psNode);
	void	InsertDetachedBeforeHead(M* psNode);
	void	InsertDetachedBeforeNode(M* psData, M* psPos);
	void	InsertDetachedAfterNode(M* psData, M* psPos);
	void	RemoveTail(void);
	void	Remove(M* pvData);
	BOOL	SafeRemove(M* pvData);
	void	Detach(SDNode* psNodeHeader);
	void	Detach(M* pvData);
	void	FreeDetached(M* pvData);
	int		NumElements(void);
	int		IndexOf(M* pvData);
	BOOL	IsInList(M* pvData);

	//Link list insertion functions.
void InsertLinkListAfterTail(__CLinkListTemplate<M>* pcLinkList);
void InsertLinkListBeforeHead(__CLinkListTemplate<M>* pcLinkList);
void InsertLinkListAfterNode(__CLinkListTemplate<M>* pcLinkList, M* psPos);
void InsertLinkListBeforeNode(__CLinkListTemplate<M>* pcLinkList, M* psPos);

	//Sorting functions.
	void	Swap(M* psData1, M* psData2);
	void	BubbleSort(int(*)(const void*, const void*));
	void	InsertIntoSorted(int(*)(const void*, const void*), M* psNode);

protected:	
	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t iMemSize);
	void	Free(void* pv);
};


template<class M>
class CLinkListTemplate : public __CLinkListTemplate<M>
{
public:
	int		miElementSize;

	void	Init(void);
	M*		AllocateDetached(void);
	M* 		InsertAfterTail(void);
	M* 		InsertBeforeHead(void);
	M*		InsertBeforeNode(M* psPos);
	M*		InsertAfterNode(M* psPos);
	M*		Add(void);
	int		ByteSize(void);
};


#define CLinkListTemplateDataGetHeader(pvData)		DataGetHeaderMacro<SDNode, M>(pvData)
#define CLinkListTemplateHeaderGetData(pvHeader)	HeaderGetDataMacro<SDNode, M>(pvHeader)



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* __CLinkListTemplate<M>::Malloc(size_t tSize)
{
	return malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::Free(void* pv)
{
	free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* __CLinkListTemplate<M>::Realloc(void* pv, size_t tSize)
{
	pv = realloc(pv, tSize);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::Kill(void)
{
	SDNode*	psNode;
	SDNode*	psNode2;

	psNode = mpsHead;
	while (psNode)
	{
		psNode2 = psNode->psNext;
		Free(psNode);
		psNode = psNode2;
	}
	mpsHead = NULL;
	mpsTail = NULL;
	miNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CLinkListTemplate<M>::GetHead(void)
{
	return CLinkListTemplateHeaderGetData(mpsHead);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CLinkListTemplate<M>::GetTail(void)
{
	return CLinkListTemplateHeaderGetData(mpsTail);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CLinkListTemplate<M>::GetNext(M* pvData)
{
	SDNode*		psNodeHeader;

	psNodeHeader = CLinkListTemplateDataGetHeader(pvData);
	if (psNodeHeader->psNext)
	{
		return CLinkListTemplateHeaderGetData(psNodeHeader->psNext);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CLinkListTemplate<M>::GetPrev(M* pvData)
{
	SDNode*		psNodeHeader;

	psNodeHeader = CLinkListTemplateDataGetHeader(pvData);
	if (psNodeHeader->psPrev)
	{
		return CLinkListTemplateHeaderGetData(psNodeHeader->psPrev);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CLinkListTemplate<M>::Get(int iNum)
{
	int	iCount;
	M*	psData;

	psData = GetHead();
	for (iCount = 0;;iCount++)
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
template<class M>
void __CLinkListTemplate<M>::InsertDetachedAfterTail(M* psData)
{
	SDNode*		psNode;

	psNode = CLinkListTemplateDataGetHeader(psData);
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
	miNumElements++;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::InsertDetachedBeforeHead(M* psData)
{
	SDNode*		psNode;

	psNode = CLinkListTemplateDataGetHeader(psData);
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
	miNumElements++;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::InsertDetachedBeforeNode(M* psDataNode, M* psDataPos)
{
	SDNode*		psPos;
	SDNode*		psNode;

	psNode = CLinkListTemplateDataGetHeader(psDataNode);
	psPos = CLinkListTemplateDataGetHeader(psDataPos);

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
	miNumElements++;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::InsertDetachedAfterNode(M* psDataNode, M* psData)
{
	SDNode*		psPos;
	SDNode*		psNode;

	psNode = CLinkListTemplateDataGetHeader(psDataNode);
	psPos = CLinkListTemplateDataGetHeader(psData);

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
	miNumElements++;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::RemoveTail(void)
{
	Remove(GetTail());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::Remove(M* psNodeData)
{	
	Detach(psNodeData);
	FreeDetached(psNodeData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CLinkListTemplate<M>::SafeRemove(M* pvData)
{	
	if (IsInList(pvData))
	{
		Remove(pvData);
		return TRUE;
	}
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::Detach(SDNode* psNodeHeader)
{
	if (psNodeHeader)
	{
		if (psNodeHeader->psPrev)
		{
			psNodeHeader->psPrev->psNext = psNodeHeader->psNext;
		}
		else
		{
			mpsHead = psNodeHeader->psNext;
		}
		if (psNodeHeader->psNext)
		{
			psNodeHeader->psNext->psPrev = psNodeHeader->psPrev;
		}
		else
		{
			mpsTail = psNodeHeader->psPrev;
		}
		miNumElements--;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::Detach(M* psNodeData)
{
	SDNode*		psNodeHeader;

	psNodeHeader = CLinkListTemplateDataGetHeader(psNodeData);
	Detach(psNodeHeader);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::FreeDetached(M* psNodeData)
{
	SDNode*		psNodeHeader;

	psNodeHeader = CLinkListTemplateDataGetHeader(psNodeData);
	if (psNodeHeader)
	{
		Free(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::Swap(M* psData1, M* psData2)
{
	SDNode*		psNode1;
	SDNode*		psNode2;
	SDNode		sNodeTemp1;
	SDNode		sNodeTemp2;

	psNode1 = CLinkListTemplateDataGetHeader(psData1);
	psNode2 = CLinkListTemplateDataGetHeader(psData2);

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
template<class M>
int	__CLinkListTemplate<M>::NumElements(void)
{
	return miNumElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
//
//	!!!!!!!!!! CAUTION: Not optimised !!!!!!!!!!
//
template<class M>
void __CLinkListTemplate<M>::BubbleSort(int(* Func)(const void*, const void*))
{
	M*		pcCurr;
	M*		pcNext;
	int		iResult;
	BOOL	bSwapped;

	pcCurr = GetHead();
	if (!pcCurr)
	{
		return;
	}

	bSwapped = TRUE;
	while (bSwapped)
	{
		pcCurr = GetHead();
		pcNext = GetNext(pcCurr);
		bSwapped = FALSE;

		while ((pcNext) && (pcCurr))
		{
			iResult = Func(pcCurr, pcNext);
			if (iResult > 0)
			{
				Swap(pcCurr, pcNext);
				bSwapped = TRUE;
			}
			else
			{
				pcCurr = GetNext(pcCurr);
			}
			if (pcCurr)
			{
				pcNext = GetNext(pcCurr);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::InsertIntoSorted(int(* Func)(const void*, const void*), M* psData)
{
	M*			pcCurr;
	int			iResult;

	pcCurr = GetHead();

	for(;;)
	{
		iResult = Func(psData, pcCurr);
		if (iResult < 0)
		{
			InsertDetachedBeforeNode(psData, pcCurr);
			break;
		}
		pcCurr = GetNext(pcCurr);
		if (pcCurr == NULL)
		{
			InsertDetachedAfterTail(psData);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::InsertLinkListAfterTail(__CLinkListTemplate<M>* pcLinkList)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::InsertLinkListBeforeHead(__CLinkListTemplate<M>* pcLinkList)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::InsertLinkListAfterNode(__CLinkListTemplate<M>* pcLinkList, M* psPos)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CLinkListTemplate<M>::InsertLinkListBeforeNode(__CLinkListTemplate<M>* pcLinkList, M* psPos)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CLinkListTemplate<M>::IsInList(M* pvData)
{
	int iIndex;

	iIndex = IndexOf(pvData);
	return iIndex != -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CLinkListTemplate<M>::IndexOf(M* pvData)
{
	int iIndex;
	M*	pvNode;

	iIndex = 0;
	pvNode = GetHead();
	while (pvNode)
	{
		if (pvNode == pvData)
		{
			return iIndex;
		}
		iIndex++;
		pvNode = GetNext(pvNode);
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CLinkListTemplate<M>::Init(void)
{
	mpsHead = NULL;
	mpsTail = NULL;
	miNumElements = 0;
	miElementSize = sizeof(M);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkListTemplate<M>::AllocateDetached(void)
{
	SDNode*		psNode;

	psNode = (SDNode*)Malloc(sizeof(SDNode) + miElementSize);
	return CLinkListTemplateHeaderGetData(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkListTemplate<M>::InsertAfterTail(void)
{
	SDNode*		psNode;
	M*			psData;

	psNode = (SDNode*)Malloc(sizeof(SDNode) + miElementSize);
	psData = CLinkListTemplateHeaderGetData(psNode);
	InsertDetachedAfterTail(psData);
	return psData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkListTemplate<M>::InsertBeforeHead(void)
{
	SDNode*		psNode;
	M*			psData;

	psNode = (SDNode*)Malloc(sizeof(SDNode) + miElementSize);
	psData = CLinkListTemplateHeaderGetData(psNode);
	InsertDetachedBeforeHead(psData);
	return psData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkListTemplate<M>::InsertBeforeNode(M* psPos)
{
	SDNode*		psNode;
	M*			psData;

	psNode = (SDNode*)Malloc(sizeof(SDNode) + miElementSize);
	psData = CLinkListTemplateHeaderGetData(psNode);
	InsertDetachedBeforeNode(psData, psPos);
	return psData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkListTemplate<M>::InsertAfterNode(M* psPos)
{
	SDNode*		psNode;
	M*			psData;

	psNode = (SDNode*)Malloc(sizeof(SDNode) + miElementSize);
	psData = CLinkListTemplateHeaderGetData(psNode);
	InsertDetachedAfterNode(psData, psPos);
	return psData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CLinkListTemplate<M>::Add(void)
{
	//This included to make swapping between arrays and lists easier.
	return InsertAfterTail();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CLinkListTemplate<M>::ByteSize(void)
{
	return ((sizeof(SDNode) + miElementSize) * miNumElements);
}


#endif // __TEMPLATE_LIST_H__


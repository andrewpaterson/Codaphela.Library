#include "Define.h"
#include "DataMacro.h"
#include "LinkedListBase.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBase::Malloc(size_t tSize)
{
	return malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBase::Free(void* pv)
{
	free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBase::Realloc(void* pv, size_t tSize)
{
	pv = realloc(pv, tSize);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBase::Init(void)
{
	mpsHead = NULL;
	mpsTail = NULL;
	miNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBase::Kill(void)
{
	SLLNode*	psNode;
	SLLNode*	psNode2;

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
void* CLinkedListBase::GetHead(void)
{
	return HeaderGetData<SLLNode, void>(mpsHead);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBase::GetTail(void)
{
	return HeaderGetData<SLLNode, void>(mpsTail);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBase::GetNext(void* pvData)
{
	SLLNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SLLNode, void>(pvData);
	if (psNodeHeader->psNext)
	{
		return HeaderGetData<SLLNode, void>(psNodeHeader->psNext);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBase::GetPrev(void* pvData)
{
	SLLNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SLLNode, void>(pvData);
	if (psNodeHeader->psPrev)
	{
		return HeaderGetData<SLLNode, void>(psNodeHeader->psPrev);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBase::Get(int iNum)
{
	int	iCount;
	void*	psData;

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
void CLinkedListBase::InsertDetachedAfterTail(void* psData)
{
	SLLNode*		psNode;

	psNode = DataGetHeader<SLLNode, void>(psData);
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
void CLinkedListBase::InsertDetachedBeforeHead(void* psData)
{
	SLLNode*		psNode;

	psNode = DataGetHeader<SLLNode, void>(psData);
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
void CLinkedListBase::InsertDetachedBeforeNode(void* psDataNode, void* psDataPos)
{
	SLLNode*		psPos;
	SLLNode*		psNode;

	psNode = DataGetHeader<SLLNode, void>(psDataNode);
	psPos = DataGetHeader<SLLNode, void>(psDataPos);

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
void CLinkedListBase::InsertDetachedAfterNode(void* psDataNode, void* psData)
{
	SLLNode*		psPos;
	SLLNode*		psNode;

	psNode = DataGetHeader<SLLNode, void>(psDataNode);
	psPos = DataGetHeader<SLLNode, void>(psData);

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
void CLinkedListBase::RemoveTail(void)
{
	Remove(GetTail());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBase::Remove(void* psNodeData)
{	
	Detach(psNodeData);
	FreeDetached(psNodeData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBase::SafeRemove(void* pvData)
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
int	CLinkedListBase::GetNodeSize(void* pvData)
{
	SLLNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SLLNode, void>(pvData);
	return psNodeHeader->iSize + sizeof(SLLNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkedListBase::ByteSize(void)
{
	int		iSize;
	void*	pvNode;

	iSize = 0;

	pvNode = GetHead();
	while(pvNode)
	{
		iSize += GetNodeSize(pvNode);
		pvNode = GetNext(pvNode);
	}

	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBase::InsertAfterTail(int iDataSize)
{
	void*			pvData;

	pvData = AllocateDetached(iDataSize);
	InsertDetachedAfterTail(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBase::InsertBeforeHead(int iDataSize)
{
	void*			pvData;

	pvData = AllocateDetached(iDataSize);
	InsertDetachedBeforeHead(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBase::InsertBeforeNode(int iDataSize, void* psPos)
{
	void*			pvData;

	pvData = AllocateDetached(iDataSize);
	InsertDetachedBeforeNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBase::InsertAfterNode(int iDataSize, void* psPos)
{
	void*			pvData;

	pvData = AllocateDetached(iDataSize);
	InsertDetachedAfterNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBase::Detach(SLLNode* psNodeHeader)
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
void CLinkedListBase::Detach(void* psNodeData)
{
	SLLNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SLLNode, void>(psNodeData);
	Detach(psNodeHeader);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBase::AllocateDetached(int iDataSize)
{
	SLLNode*		psNode;

	psNode = (SLLNode*)Malloc(sizeof(SLLNode) + iDataSize);
	psNode->iSize = iDataSize;
	return HeaderGetData<SLLNode, void>(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBase::FreeDetached(void* psNodeData)
{
	SLLNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SLLNode, void>(psNodeData);
	if (psNodeHeader)
	{
		Free(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBase::Swap(void* psData1, void* psData2)
{
	SLLNode*		psNode1;
	SLLNode*		psNode2;
	SLLNode		sNodeTemp1;
	SLLNode		sNodeTemp2;

	psNode1 = DataGetHeader<SLLNode, void>(psData1);
	psNode2 = DataGetHeader<SLLNode, void>(psData2);

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
int	CLinkedListBase::NumElements(void)
{
	return miNumElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBase::BubbleSort(int(* Func)(const void*, const void*))
{
	void*		pcCurr;
	void*		pcNext;
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
void CLinkedListBase::InsertDetachedIntoSorted(int(* Func)(const void*, const void*), void* psData)
{
	void*			pcCurr;
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
BOOL CLinkedListBase::IsInList(void* pvData)
{
	int iIndex;

	iIndex = IndexOf(pvData);
	return iIndex != -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkedListBase::IndexOf(void* pvData)
{
	int iIndex;
	void*	pvNode;

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
BOOL CLinkedListBase::WriteLinkListBlock(CFileWriter* pcFileWriter)
{
	void*			pvData;
	int				iSize;

	if (!pcFileWriter->WriteData(this, sizeof(CLinkedListBase))) 
	{ 
		return FALSE; 
	}

	pvData = GetHead();
	while (pvData)
	{
		iSize = GetNodeSize(pvData);
		if (!pcFileWriter->WriteInt(iSize)) 
		{ 
			return FALSE; 
		}
		if (!pcFileWriter->WriteData(pvData, iSize)) 
		{ 
			return FALSE; 
		}

		pvData = GetNext(pvData);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBase::ReadLinkListBlock(CFileReader* pcFileReader)
{
	int				iNumElements;
	int				i;
	void*			pvData;
	int				iSize;

	if (!pcFileReader->ReadData(this, sizeof(CLinkedListBase))) 
	{ 
		return FALSE; 
	}

	iNumElements = NumElements();
	Init();
	for (i = 0; i < iNumElements; i++)
	{
		if (!pcFileReader->ReadInt(&iSize)) 
		{ 
			return FALSE; 
		}

		pvData = InsertAfterTail(iSize);
		if (!pcFileReader->ReadData(pvData, iSize)) 
		{ 
			return FALSE; 
		}
	}
	return TRUE;
}


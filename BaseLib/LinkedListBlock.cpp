#include "Define.h"
#include "DataMacro.h"
#include "SystemAllocator.h"
#include "GlobalMemory.h"
#include "LinkedListBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::Malloc(size_t tSize)
{
	return mpcMalloc->Malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::Free(void* pv)
{
	mpcMalloc->Free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::Realloc(void* pv, size_t tSize)
{
	return mpcMalloc->Realloc(pv, tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::Init(CMallocator* pcMalloc)
{
	mpcMalloc = pcMalloc;
	mcList.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::Kill(void)
{
	SLLNode*	psNode;
	SLLNode*	psNode2;

	psNode = mcList.GetHead();
	while (psNode)
	{
		psNode2 = mcList.GetNext(psNode);
		Free(psNode);
		psNode = psNode2;
	}
	mcList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::GetHead(void)
{
	return NodeGetData(mcList.GetHead());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::GetTail(void)
{
	return NodeGetData(mcList.GetTail());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::GetNext(void* pvData)
{
	SLLBlockNode*	psNode;

	psNode = DataGetNode(pvData);
	psNode = (SLLBlockNode*)mcList.GetNext(psNode);
	if (psNode)
	{
		return NodeGetData(psNode);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::GetPrev(void* pvData)
{
	SLLBlockNode*		psNode;

	psNode = DataGetNode(pvData);
	psNode = (SLLBlockNode*)mcList.GetPrev(psNode);
	if (psNode)
	{
		return NodeGetData(psNode);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::Get(int iNum)
{
	SLLBlockNode* psNodeHeader;

	psNodeHeader = (SLLBlockNode*)mcList.Get(iNum);
	return HeaderGetData<SLLBlockNode, void>(psNodeHeader);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::InsertDetachedAfterTail(void* psData)
{
	SLLBlockNode*		psNode;

	psNode = DataGetNode(psData);
	mcList.InsertAfterTail(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::InsertDetachedBeforeHead(void* psData)
{
	SLLBlockNode*		psNode;

	psNode = DataGetNode(psData);
	mcList.InsertBeforeHead(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::InsertDetachedBeforeNode(void* psDataNode, void* psDataPos)
{
	SLLBlockNode*		psPos;
	SLLBlockNode*		psNode;

	psNode = DataGetNode(psDataNode);
	psPos = DataGetNode(psDataPos);

	mcList.InsertAfterNode(psPos, psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::InsertDetachedAfterNode(void* psDataNode, void* psData)
{
	SLLBlockNode*		psPos;
	SLLBlockNode*		psNode;

	psNode = DataGetNode(psDataNode);
	psPos = DataGetNode(psData);

	mcList.InsertAfterNode(psPos, psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::RemoveTail(void)
{
	Remove(GetTail());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::Remove(void* psNodeData)
{	
	Detach(psNodeData);
	FreeDetached(psNodeData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlock::SafeRemove(void* pvData)
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
unsigned int CLinkedListBlock::GetNodeSize(void* pvData)
{
	SLLBlockNode*		psNodeHeader;

	psNodeHeader = DataGetNode(pvData);
	return psNodeHeader->uiSize + sizeof(SLLBlockNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::NodeGetData(SLLNode* psNode)
{
	return HeaderGetData<SLLBlockNode, void>((SLLBlockNode*)psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SLLBlockNode* CLinkedListBlock::DataGetNode(void* pvData)
{
	return DataGetHeader<SLLBlockNode, void>(pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkedListBlock::ByteSize(void)
{
	int		iSize;
	void*	pvNode;

	iSize = 0;

	pvNode = GetHead();
	while (pvNode)
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
void* CLinkedListBlock::InsertAfterTail(unsigned int uiDataSize)
{
	void*			pvData;

	pvData = AllocateDetached(uiDataSize);
	InsertDetachedAfterTail(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::InsertBeforeHead(unsigned int uiDataSize)
{
	void*			pvData;

	pvData = AllocateDetached(uiDataSize);
	InsertDetachedBeforeHead(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::InsertBeforeNode(unsigned int uiDataSize, void* psPos)
{
	void*			pvData;

	pvData = AllocateDetached(uiDataSize);
	InsertDetachedBeforeNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::InsertAfterNode(unsigned int uiDataSize, void* psPos)
{
	void*			pvData;

	pvData = AllocateDetached(uiDataSize);
	InsertDetachedAfterNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::Detach(SLLBlockNode* psNodeHeader)
{
	mcList.Remove(psNodeHeader);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::Detach(void* psNodeData)
{
	SLLBlockNode*		psNodeHeader;

	psNodeHeader = DataGetNode(psNodeData);
	mcList.Remove(psNodeHeader);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::AllocateDetached(unsigned int uiDataSize)
{
	SLLBlockNode*		psNode;

	psNode = (SLLBlockNode*)Malloc(sizeof(SLLBlockNode) + uiDataSize);
	psNode->uiSize = uiDataSize;
	psNode->psNext = NULL;
	psNode->psPrev = NULL;
	return NodeGetData(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::FreeDetached(void* psNodeData)
{
	SLLBlockNode*		psNodeHeader;

	psNodeHeader = DataGetNode(psNodeData);
	if (psNodeHeader)
	{
		Free(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::Swap(void* psData1, void* psData2)
{
	SLLBlockNode*	psNode1;
	SLLBlockNode*	psNode2;

	psNode1 = DataGetNode(psData1);
	psNode2 = DataGetNode(psData2);

	mcList.Swap(psNode1, psNode2);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int	CLinkedListBlock::NumElements(void)
{
	int		iCount;
	void*	pvNode;

	iCount = 0;
	pvNode = GetHead();
	while (pvNode)
	{
		iCount++;
		pvNode = GetNext(pvNode);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::BubbleSort(int(*Func)(const void*, const void*))
{
	mcList.BubbleSort(Func, sizeof(SLLBlockNode));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::InsertDetachedIntoSorted(int(* Func)(const void*, const void*), void* pvData)
{
	SLLBlockNode*	psNode;

	psNode = DataGetNode(pvData);
	mcList.InsertIntoSorted(Func, psNode, sizeof(SLLBlockNode));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlock::IsInList(void* pvData)
{
	SLLBlockNode* psNode;

	psNode = DataGetNode(pvData);
	return mcList.IsInList(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkedListBlock::IndexOf(void* pvData)
{
	SLLBlockNode*	psNode;

	psNode = DataGetNode(pvData);
	return mcList.IndexOf(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlock::WriteHeader(CFileWriter* pcFileWriter)
{
	SLinkedListBlockDesc	sHeader;
	int						iNumElements;

	iNumElements = NumElements();
	sHeader.Init(iNumElements);

	if (!pcFileWriter->WriteData(&sHeader, sizeof(SLinkedListBlockDesc)))
	{
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlock::WriteAllocatorAndHeader(CFileWriter* pcFileWriter)
{
	BOOL	bResult;

	bResult = gcMallocators.WriteMallocator(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = WriteHeader(pcFileWriter);
	if (!bResult)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlock::Write(CFileWriter* pcFileWriter)
{
	void*			pvData;
	int				iSize;

	if (!WriteAllocatorAndHeader(pcFileWriter))
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
BOOL CLinkedListBlock::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, SLinkedListBlockDesc* psDesc)
{
	if (!pcFileReader->ReadData(psDesc, sizeof(SLinkedListBlockDesc)))
	{
		return FALSE;
	}

	mcList.Init();
	mpcMalloc = pcMalloc;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlock::ReadAllocatorAndHeader(CFileReader* pcFileReader, SLinkedListBlockDesc* psDesc)
{
	BOOL			bResult;
	CMallocator*	pcMalloc;

	pcMalloc = gcMallocators.ReadMallocator(pcFileReader);
	if (pcMalloc == NULL)
	{
		return FALSE;
	}

	bResult = ReadHeader(pcFileReader, pcMalloc, psDesc);
	if (!bResult)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlock::Read(CFileReader* pcFileReader)
{
	int						iNumElements;
	int						i;
	void*					pvData;
	int						iSize;
	BOOL					bResult;
	SLinkedListBlockDesc	sDesc;

	bResult = ReadAllocatorAndHeader(pcFileReader, &sDesc);
	if (!bResult)
	{
		return FALSE;
	}

	iNumElements = sDesc.iNumElements;
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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SLinkedListBlockDesc::Init(int iNumElements)
{
	this->iNumElements = iNumElements;
}


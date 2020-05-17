#include "Define.h"
#include "DataMacro.h"
#include "SystemAllocator.h"
#include "GlobalMemory.h"
#include "LinkedListBlock.h"


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
	CBaseLinkedListBlock::Init(pcMalloc, sizeof(SLLBlockNode));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlock::Kill(void)
{
	CBaseLinkedListBlock::Kill();
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
unsigned int CLinkedListBlock::GetNodeSize(void* pvData)
{
	SLLBlockNode*		psNodeHeader;

	psNodeHeader = (SLLBlockNode*)DataGetNode(pvData);
	return psNodeHeader->uiSize + muiNodeSize;
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
	SLLBlockNode*	psNode;

	psNode = AllocateDetached(uiDataSize);
	return CBaseLinkedListBlock::InsertDetachedAfterTail(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::InsertBeforeHead(unsigned int uiDataSize)
{
	SLLBlockNode* psNode;

	psNode = AllocateDetached(uiDataSize);
	return CBaseLinkedListBlock::InsertDetachedBeforeHead(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::InsertBeforeNode(unsigned int uiDataSize, void* psPos)
{
	SLLBlockNode* psNode;
	SLLBlockNode* psNodePos;

	psNodePos = DataGetNode(psPos);
	psNode = AllocateDetached(uiDataSize);
	return CBaseLinkedListBlock::InsertDetachedBeforeNode(psNode, psNodePos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::InsertAfterNode(unsigned int uiDataSize, void* psPos)
{
	SLLBlockNode* psNode;
	SLLBlockNode* psNodePos;

	psNodePos = DataGetNode(psPos);
	psNode = AllocateDetached(uiDataSize);
	return CBaseLinkedListBlock::InsertDetachedAfterNode(psNode, psNodePos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SLLBlockNode* CLinkedListBlock::AllocateDetached(unsigned int uiDataSize)
{
	SLLBlockNode*		psNode;

	psNode = (SLLBlockNode*)CBaseLinkedListBlock::AllocateDetached(uiDataSize);
	psNode->uiSize = uiDataSize;
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SLLBlockNode* CLinkedListBlock::DataGetNode(void* pvData)
{
	return (SLLBlockNode*)CBaseLinkedListBlock::DataGetNode(pvData);
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
void CLinkedListBlock::InsertDetachedAfterTail(void* pvData)
{
	SLLBlockNode* psNode;

	psNode = DataGetNode(pvData);
	CBaseLinkedListBlock::InsertDetachedAfterTail(psNode);
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
	sHeader.Init(iNumElements, muiNodeSize);

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
BOOL CLinkedListBlock::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, int* piNumElements)
{
	SLinkedListBlockDesc	sDesc;

	if (!pcFileReader->ReadData(&sDesc, sizeof(SLinkedListBlockDesc)))
	{
		return FALSE;
	}

	CBaseLinkedListBlock::Init(pcMalloc, sDesc.uiNodeSize);
	*piNumElements = sDesc.iNumElements;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlock::ReadAllocatorAndHeader(CFileReader* pcFileReader, int* piNumElements)
{
	BOOL			bResult;
	CMallocator*	pcMalloc;

	pcMalloc = gcMallocators.ReadMallocator(pcFileReader);
	if (pcMalloc == NULL)
	{
		return FALSE;
	}

	bResult = ReadHeader(pcFileReader, pcMalloc, piNumElements);
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

	bResult = ReadAllocatorAndHeader(pcFileReader, &iNumElements);
	if (!bResult)
	{
		return FALSE;
	}

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


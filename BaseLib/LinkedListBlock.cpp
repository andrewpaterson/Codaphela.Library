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
	SLLBlockNode* psNode;

	psNode = AllocateDetached(uiDataSize);
	return CBaseLinkedListBlock::InsertDetachedAfterTail(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlock::Add(unsigned int uiDataSize)
{
	return InsertAfterTail(uiDataSize);
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
bool CLinkedListBlock::Write(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = gcMallocators.Write(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return false;
	}

	bResult = WriteHeader(pcFileWriter);
	if (!bResult)
	{
		return false;
	}

	bResult = WriteData(pcFileWriter);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CLinkedListBlock::WriteHeader(CFileWriter* pcFileWriter)
{
	SLinkedListBlockDesc	sHeader;
	int						iNumElements;

	iNumElements = NumElements();
	sHeader.Init(iNumElements, muiNodeSize);

	if (!pcFileWriter->WriteData(&sHeader, sizeof(SLinkedListBlockDesc)))
	{
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CLinkedListBlock::WriteData(CFileWriter* pcFileWriter)
{
	void*	pvData;
	int		iSize;

	pvData = GetHead();
	while (pvData)
	{
		iSize = GetNodeSize(pvData);
		if (!pcFileWriter->WriteInt(iSize))
		{
			return false;
		}
		if (!pcFileWriter->WriteData(pvData, iSize))
		{
			return false;
		}

		pvData = GetNext(pvData);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CLinkedListBlock::Read(CFileReader* pcFileReader)
{
	//Do not call .Init() before Read().

	int				iNumElements;
	bool			bResult;
	CMallocator*	pcMalloc;

	pcMalloc = gcMallocators.Read(pcFileReader);
	if (pcMalloc == NULL)
	{
		return false;
	}

	bResult = ReadHeader(pcFileReader, pcMalloc, &iNumElements);
	if (!bResult)
	{
		return false;
	}

	bResult = ReadData(pcFileReader, iNumElements);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CLinkedListBlock::ReadHeader(CFileReader* pcFileReader, CMallocator* pcMalloc, int* piNumElements)
{
	SLinkedListBlockDesc	sDesc;

	if (!pcFileReader->ReadData(&sDesc, sizeof(SLinkedListBlockDesc)))
	{
		return false;
	}

	if (sizeof(SLLBlockNode) != sDesc.uiNodeSize)
	{
		return false;
	}

	Init(pcMalloc);
	*piNumElements = sDesc.iNumElements;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CLinkedListBlock::ReadData(CFileReader* pcFileReader, int iNumElements)
{
	int		i;
	void*	pvData;
	int		iSize;

	for (i = 0; i < iNumElements; i++)
	{
		if (!pcFileReader->ReadInt(&iSize))
		{
			return false;
		}

		pvData = InsertAfterTail(iSize);
		if (!pcFileReader->ReadData(pvData, iSize))
		{
			return false;
		}
	}
	return true;
}


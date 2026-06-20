#include "Logger.h"
#include "DataMacro.h"
#include "GlobalMemory.h"
#include "StackMemory.h"
#include "MapHelper.h"
#include "SetBlock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::_Init(void)
{
	mapArray._Init();
	miLargestDataSize = 0;
	mbOverwrite = false;
	mpcDataFree = NULL;
	mpcDataIO = this;
	mfCompare = NULL;
	mpcMalloc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::Init(bool bOverwrite)
{
	Init(&gcSystemAllocator, NULL, &CompareSNodeData, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::Init(CMallocator* pcMalloc, bool bOverwrite)
{
	Init(pcMalloc, NULL, &CompareSNodeData, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::Init(DataCompare fCompare, bool bOverwrite)
{
	Init(&gcSystemAllocator, fCompare, &CompareSNode, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::Init(CMallocator* pcMalloc, DataCompare fCompare, bool bOverwrite)
{
	Init(pcMalloc, fCompare, &CompareSNode, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::Init(CMallocator* pcMalloc, DataCompare fCompare, DataCompare fNodeCompare, bool bOverwrite)
{
	size	iHoldingBufferSize;
	size	iHoldingBuffers;

	CMalloc::Init(pcMalloc);

	iHoldingBufferSize = 256;
	iHoldingBuffers = 4;

	mfCompare = fCompare;
	mapArray.Init(pcMalloc, sizeof(void*), iHoldingBufferSize, iHoldingBuffers, fNodeCompare);
	miLargestDataSize = 0;
	mbOverwrite = bOverwrite;
	mpcDataFree = NULL;
	mpcDataIO = this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::Kill(void)
{
	size		i;
	SSNode**	ppsNode;
	size		uiNumElements;

	mapArray.InsertHoldingIntoSorted();
	uiNumElements = mapArray.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		ppsNode = (SSNode**)mapArray.GetInSorted(i);
		FreeNode(*ppsNode);
	}

	miLargestDataSize = 0;
	mapArray.Kill();
	CMalloc::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::ReInit(void)
{
	CMallocator*		pcMalloc;
	DataCompare			fCompare;
	DataCompare			fNodeCompare;
	bool				bOverwrite;
	CDataFree*			pcDataFree;
	CDataIO*			pcDataIO;

	fNodeCompare = mapArray.GetCompare();
	pcMalloc = mpcMalloc;
	fCompare = mfCompare;
	bOverwrite = mbOverwrite;
	pcDataFree = mpcDataFree;
	pcDataIO = mpcDataIO;

	Kill();
	Init(pcMalloc, fCompare, fNodeCompare, bOverwrite);
	SetDataFreeCallback(pcDataFree);
	SetDataIOCallback(pcDataIO);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::FreeNode(SSNode* psNode)
{
	if (mpcDataFree)
	{
		mpcDataFree->FreeData(psNode);
	}
	mpcMalloc->Free(psNode);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSetBlock::Get(void* pvData, size iDataSize)
{
	SSNode*		psNode;

	psNode = GetNode(pvData, iDataSize);
	if (!psNode)
	{
		return NULL;
	}

	pvData = GetData(psNode);

	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSetBlock::Put(void* pvData, size iDataSize)
{
	SSNode*		psNode;
	void*		pvDestData;
	bool		bResult;
	void*		pvExistingData;

	if (iDataSize > MAX_KEY_SIZE)
	{
		gcLogger.Error2(__METHOD__, " Data too long.  Cannot Put.", NULL);
		return NULL;
	}

	//This is really not optimal as it will have to find the node twice and may reallocate a new node with the same size as the old one.
	if (mbOverwrite)
	{
		//It's safe to call remove if the key does not exist.
		Remove(pvData, iDataSize);
	}
	else
	{
		pvExistingData = Get(pvData, iDataSize);
		if (pvExistingData)
		{
			gcLogger.Error2(__METHOD__, " Data already exists.  Cannot Put.", NULL);
			return NULL;
		}
	}

	psNode = AllocateNode(iDataSize);
	if (!psNode)
	{
		return NULL;
	}

	pvDestData = RemapSinglePointer(psNode, sizeof(SSNode));
	memcpy(pvDestData, pvData, iDataSize);

	bResult = mapArray.Add(&psNode);
	if (bResult)
	{
		return pvDestData;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::Remove(void* pvData, size iDataSize)
{
	SSNode*		psNode;

	psNode = GetNode(pvData, iDataSize);
	if (psNode)
	{
		mapArray.Remove(&psNode);
		FreeNode(psNode);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::Has(void* pvData, size iDataSize)
{
	SSNode* psNode;

	psNode = GetNode(pvData, iDataSize);
	return psNode != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSNode* CSetBlock::GetNode(void* pvData, size iDataSize)
{
	SSNode**	ppsNode;
	SSNode*		psSourceNode;
	void*		pvSourceData;
	char		ac[MAX_KEY_SIZE + sizeof(SSNode)];

	if (iDataSize > MAX_KEY_SIZE)
	{
		gcLogger.Error2(__METHOD__, " Data too long.  Cannot Get.", NULL);
		return NULL;
	}

	psSourceNode = (SSNode*)ac;

	psSourceNode->fCompare = mfCompare;
	psSourceNode->iDataSize = iDataSize;
	pvSourceData = RemapSinglePointer(psSourceNode, sizeof(SSNode));
	memcpy(pvSourceData, pvData, iDataSize);

	ppsNode = (SSNode**)mapArray.Get(&psSourceNode);

	if (!ppsNode)
	{
		return NULL;
	}

	return *ppsNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSetBlock::NumElements(void)
{
	return mapArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSetBlock::GetSortedSize(void)
{
	return mapArray.GetSortedSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSetBlock::GetHoldingSize(void)
{
	return mapArray.GetHoldingSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::StartIteration(SSetIterator* psIterator, void** ppvData, size* piDataSize)
{
	SSNode**	ppsNode;
	void*		pvData;
	SSNode*		psNode;

	ppsNode = (SSNode**)mapArray.StartIteration(psIterator);
	if (!ppsNode)
	{
		return false;
	}

	psNode = *ppsNode;
	pvData = RemapSinglePointer(psNode, sizeof(SSNode));
	SafeAssign(ppvData, pvData);
	SafeAssign(piDataSize, psNode->iDataSize);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::Iterate(SSetIterator* psIterator, void** ppvData, size* piDataSize)
{
	void*		pvData;
	SSNode*		psNode;
	SSNode**	ppsNode;

	ppsNode = (SSNode**)mapArray.Iterate(psIterator);
	if (!ppsNode)
	{
		return false;
	}

	psNode = *ppsNode;
	pvData = RemapSinglePointer(psNode, sizeof(SSNode));
	SafeAssign(ppvData, pvData);
	SafeAssign(piDataSize, psNode->iDataSize);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSNode* CSetBlock::GetNode(SSetIterator* psIterator)
{
	SSNode** ppsNode;

	ppsNode = (SSNode**)mapArray.GetIterated(psIterator);
	if (ppsNode)
	{
		return *ppsNode;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::WriteSetBlockHeader(CFileWriter* pcFileWriter)
{
	bool	bResult;

	InsertHoldingIntoSorted();

	bResult = gcMallocators.Write(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return false;
	}

	bResult = mapArray.WriteHeader(pcFileWriter);
	if (!bResult)
	{
		return false;
	}

	bResult = mapArray.GetSortedArray()->WriteHeader(pcFileWriter);
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::ReadSetBlockHeader(CFileReader* pcFileReader, DataCompare fCompare, CDataIO* pcDataIO, CDataFree* pcDataFree)
{
	CMallocator*	pcMalloc;
	bool			bResult;

	if (fCompare == NULL)
	{
		return false;
	}

	pcMalloc = gcMallocators.Read(pcFileReader);
	if (pcMalloc == NULL)
	{
		return false;
	}

	bResult = mapArray.ReadHeader(pcMalloc, pcFileReader, &CompareSNode);
	if (!bResult)
	{
		return false;
	}

	bResult = mapArray.GetSortedArray()->ReadHeader(pcFileReader, pcMalloc);
	if (!bResult)
	{
		return false;
	}

	CMalloc::Init(pcMalloc);

	mpcDataFree = pcDataFree;
	mpcDataIO = pcDataIO;
	mfCompare = fCompare;
	miLargestDataSize = 0;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSNode* CSetBlock::WriteSizes(CFileWriter* pcFileWriter, size iIndex)
{
	SSNode*		psNode;

	psNode = *((SSNode**)mapArray.GetInSorted(iIndex));
	if (psNode)
	{
		if (!pcFileWriter->WriteSize(psNode->iDataSize))
		{
			return NULL;
		}
	}
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSNode* CSetBlock::ReadSizes(CFileReader* pcFileReader, size iIndex)
{
	SSNode*		psNode;
	SSNode**	ppsNode;
	size		iDataSize;

	if (!pcFileReader->ReadSize(&iDataSize))
	{
		return NULL;
	}
	psNode = AllocateNode(iDataSize);
	if (!psNode)
	{
		return NULL;
	}
	ppsNode = (SSNode**)mapArray.GetInSorted(iIndex);
	*ppsNode = psNode;

	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::Write(CFileWriter* pcFileWriter)
{
	bool	bResult;

	bResult = WriteSetBlockHeader(pcFileWriter);
	if (!bResult)
	{
		return false;
	}

	return WriteSetBlockElements(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::WriteSetBlockElements(CFileWriter* pcFileWriter)
{
	size		iNumElements;
	size		i;
	bool		bResult;
	SSNode*		psNode;

	iNumElements = mapArray.GetSortedSize();
	for (i = 0; i < iNumElements; i++)
	{
		psNode = WriteSizes(pcFileWriter, i);
		if (!psNode)
		{
			return false;
		}

		bResult = mpcDataIO->WriteData(pcFileWriter, psNode);
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::Read(CFileReader* pcFileReader)
{
	return Read(pcFileReader, &CompareSNodeData, this, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::Read(CFileReader* pcFileReader, DataCompare fCompare)
{
	return Read(pcFileReader, fCompare, this, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::Read(CFileReader* pcFileReader, DataCompare fCompare, CDataIO* pcDataIO, CDataFree* pcDataFree)
{
	bool		bResult;

	bResult = ReadSetBlockHeader(pcFileReader, fCompare, pcDataIO, pcDataFree);
	if (!bResult)
	{
		return false;
	}
	
	return ReadSetBlockElements(pcFileReader);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::ReadSetBlockElements(CFileReader* pcFileReader)
{
	size		i;
	size		iNumElements;
	SSNode*		psNode;
	bool		bResult;

	
	iNumElements = mapArray.GetSortedSize();
	for (i = 0; i < iNumElements; i++)
	{
		psNode = ReadSizes(pcFileReader, i);
		if (!psNode)
		{
			return false;
		}

		bResult = mpcDataIO->ReadData(pcFileReader, psNode);
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::WriteData(CFileWriter* pcFileWriter, void* pvData)
{
	bool		bResult;
	SSNode*		psNode;
	
	psNode = (SSNode*)pvData;
	pvData = RemapSinglePointer(psNode, sizeof(SSNode));
	bResult = pcFileWriter->WriteData(pvData, psNode->iDataSize);
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::ReadData(CFileReader* pcFileReader, void* pvData)
{
	SSNode*		psNode;
	bool		bResult;

	psNode = (SSNode*)pvData;
	pvData = RemapSinglePointer(psNode, sizeof(SSNode));
	bResult = pcFileReader->ReadData(pvData, psNode->iDataSize);
	if (!bResult)
	{
		return NULL;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SSNode* CSetBlock::AllocateNode(size iDataSize)
{
	SSNode*	psNode;

	psNode = (SSNode*)mpcMalloc->Malloc(sizeof(SSNode) + iDataSize);
	if (!psNode)
	{
		return NULL;
	}

	psNode->iDataSize = iDataSize;
	psNode->fCompare = mfCompare;

	if (psNode->iDataSize > miLargestDataSize)
	{
		miLargestDataSize = psNode->iDataSize;
	}
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBlockSorted* CSetBlock::GetArray(void)
{
	return &mapArray;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::SetDataFreeCallback(CDataFree* pcDataFree)
{
	mpcDataFree = pcDataFree;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::SetDataIOCallback(CDataIO* pcDataIO)
{
	mpcDataIO = pcDataIO;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::InsertHoldingIntoSorted(void)
{
	mapArray.InsertHoldingIntoSorted();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::GetInSorted(size iIndex, void** ppvData)
{
	SSNode**	ppsNode;
	void*		pvData;

	ppsNode = (SSNode**)mapArray.GetInSorted(iIndex);
	if (ppsNode)
	{
		pvData = RemapSinglePointer(*ppsNode, sizeof(SSNode));

		*ppvData = pvData;
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSetBlock::GetDataInSorted(size iIndex)
{
	SSNode**	ppsNode;
	void*		pvData;

	ppsNode = (SSNode**)mapArray.GetInSorted(iIndex);
	if (ppsNode)
	{
		pvData = RemapSinglePointer(*ppsNode, sizeof(SSNode));
		return pvData;
	}
	else
	{
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::PutInSorted(size iIndex, void** ppvData, size uiDataSize)
{
	SSNode**	ppsNode;
	void*		pvData;

	ppsNode = (SSNode**)mapArray.GetInSorted(iIndex);
	if (ppsNode)
	{
		*ppsNode = AllocateNode(uiDataSize);

		pvData = RemapSinglePointer(*ppsNode, sizeof(SSNode));
		*ppvData = pvData;
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CSetBlock::GetData(SSNode* psNode)
{
	return RemapSinglePointer(psNode, sizeof(SSNode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSetBlock::ByteSize(void)
{
	SSetIterator	sIter;
	size			iDataSize;
	bool			bExists;
	size			uiByteSize;

	uiByteSize = 0;
	bExists = StartIteration(&sIter, NULL, &iDataSize);
	while (bExists)
	{
		uiByteSize += sizeof(SSNode) + iDataSize;
		bExists = Iterate(&sIter, NULL, &iDataSize);
	}
	return mapArray.ByteSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::Pack(void)
{
	InsertHoldingIntoSorted();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSetBlock::Sort(void)
{
	return mapArray.Sort();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetBlock::IsSorted(void)
{
	return mapArray.IsSorted();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetBlock::Dump(void)
{
	mapArray.Dump();
}


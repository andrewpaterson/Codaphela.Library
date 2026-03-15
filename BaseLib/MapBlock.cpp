#include "Logger.h"
#include "DataMacro.h"
#include "GlobalMemory.h"
#include "StackMemory.h"
#include "MapHelper.h"
#include "MapBlock.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlockDataIO::Init(CMapBlock* pcMap)
{
	mpcMap = pcMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::_Init(void)
{
	mapArray._Init();
	miLargestKeySize = 0;
	mbOverwrite = false;
	mpcDataFree = NULL;
	mpcDataIO = this;
	mfKeyCompare = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(bool bOverwrite)
{
	Init(&gcSystemAllocator, NULL, &CompareMNodeKey, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(CMallocator* pcMalloc, bool bOverwrite)
{
	Init(pcMalloc, NULL, &CompareMNodeKey, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(DataCompare fKeyCompare, bool bOverwrite)
{
	Init(&gcSystemAllocator, fKeyCompare, &CompareMNode, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(CMallocator* pcMalloc, DataCompare fKeyCompare, bool bOverwrite)
{
	Init(pcMalloc, fKeyCompare, &CompareMNode, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(CMallocator* pcMalloc, DataCompare fKeyCompare, DataCompare fCompare, bool bOverwrite)
{
	size	iHoldingBufferSize;
	size	iHoldingBuffers;

	CMalloc::Init(pcMalloc);

	iHoldingBufferSize = 256;
	iHoldingBuffers = 4;

	mfKeyCompare = fKeyCompare;
	mapArray.Init(pcMalloc, sizeof(void*), iHoldingBufferSize, iHoldingBuffers, fCompare);
	miLargestKeySize = 0;
	mbOverwrite = bOverwrite;
	mpcDataFree = NULL;
	mpcDataIO = this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Kill(void)
{
	size		i;
	SMNode**	ppsNode;
	size		uiNumElements;

	mapArray.InsertHoldingIntoSorted();
	uiNumElements = mapArray.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		ppsNode = (SMNode**)mapArray.GetInSorted(i);
		FreeNode(*ppsNode);
	}

	miLargestKeySize = 0;
	mapArray.Kill();
	CMalloc::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::FreeNode(SMNode* psNode)
{
	void*	pvData;

	if (mpcDataFree)
	{
		pvData = GetValue(psNode);
		mpcDataFree->FreeData(psNode);
	}
	mpcMalloc->Free(psNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapBlock::Get(void* pvKey, size iKeySize)
{
	void*	pvData;
	bool	bFound;

	bFound = Get(pvKey, iKeySize, &pvData, NULL);
	if (bFound)
	{
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
bool CMapBlock::Get(void* pvKey, size iKeySize, void** ppvData, size* piDataSize)
{
	SMNode*		psNode;
	void*		pvData;

	psNode = GetNode(pvKey, iKeySize);
	if (!psNode)
	{
		return false;
	}

	pvData = GetValue(psNode);

	SafeAssign(ppvData, pvData);
	SafeAssign(piDataSize, psNode->iValueSize);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapBlock::Put(void* pvKey, size iKeySize, void* pvData, size iDataSize)
{
	void*	pvDestData;

	pvDestData = Put(pvKey, iKeySize, iDataSize);
	if (pvDestData != NULL)
	{
		if (pvData != NULL)
		{
			memcpy(pvDestData, pvData, iDataSize);
		}
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
void* CMapBlock::Put(void* pvKey, size iKeySize, size iDataSize)
{
	SMNode*		psNode;
	void*		pvDestKey;
	void*		pvDestData;
	bool		bResult;
	void*		pvExistingData;

	if (iKeySize > MAX_KEY_SIZE)
	{
		gcLogger.Error2(__METHOD__, " Key too long.  Cannot Put.", NULL);
		return NULL;
	}

	//This is really not optimal as it will have to find the node twice and may reallocate a new node with the same size as the old one.
	if (mbOverwrite)
	{
		//It's safe to call remove if the key does not exist.
		Remove(pvKey, iKeySize);
	}
	else
	{
		pvExistingData = Get(pvKey, iKeySize);
		if (pvExistingData)
		{
			gcLogger.Error2(__METHOD__, " Key already exists.  Cannot Put.", NULL);
			return NULL;
		}
	}

	psNode = AllocateNode(iKeySize, iDataSize);
	if (!psNode)
	{
		return NULL;
	}

	RemapKeyAndData(psNode, &pvDestKey, &pvDestData);
	memcpy(pvDestKey, pvKey, iKeySize);

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
bool CMapBlock::Remove(void* pvKey, size iKeySize)
{
	SMNode*		psNode;

	psNode = GetNode(pvKey, iKeySize);
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
size CMapBlock::DataSize(void* pvKey, size iKeySize)
{
	SMNode* psNode;

	psNode = GetNode(pvKey, iKeySize);
	if (psNode)
	{
		return psNode->iValueSize;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapBlock::HasKey(void* pvKey, size iKeySize)
{
	SMNode* psNode;

	psNode = GetNode(pvKey, iKeySize);
	return psNode != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMNode* CMapBlock::GetNode(void* pvKey, size iKeySize)
{
	SMNode**	ppsNode;
	SMNode*		psSourceNode;
	void*		pvSourceKey;
	char		ac[MAX_KEY_SIZE + sizeof(SMNode)];

	if (iKeySize > MAX_KEY_SIZE)
	{
		gcLogger.Error2(__METHOD__, " Key too long.  Cannot Get.", NULL);
		return false;
	}

	psSourceNode = (SMNode*)ac;

	psSourceNode->fKeyCompare = mfKeyCompare;
	psSourceNode->iValueSize = 0;
	psSourceNode->iKeySize = iKeySize;
	pvSourceKey = RemapSinglePointer(psSourceNode, sizeof(SMNode));
	memcpy(pvSourceKey, pvKey, iKeySize);

	ppsNode = (SMNode**)mapArray.Get(&psSourceNode);

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
size CMapBlock::NumElements(void)
{
	return mapArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapBlock::GetSortedSize(void)
{
	return mapArray.GetSortedSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapBlock::GetHoldingSize(void)
{
	return mapArray.GetHoldingSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapBlock::StartIteration(SMapIterator* psIterator, void** ppvKey, size* piKeySize, void** ppvData, size* piDataSize)
{
	SMNode**	ppsNode;
	void*		pvKey;
	void*		pvData;
	SMNode*		psNode;

	ppsNode = (SMNode**)mapArray.StartIteration(psIterator);
	if (!ppsNode)
	{
		return false;
	}

	psNode = *ppsNode;
	pvKey = RemapSinglePointer(psNode, sizeof(SMNode));
	pvData = RemapSinglePointer(pvKey, psNode->iKeySize);
	SafeAssign(ppvKey, pvKey);
	SafeAssign(ppvData, pvData);
	SafeAssign(piKeySize, psNode->iKeySize);
	SafeAssign(piDataSize, psNode->iValueSize);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapBlock::Iterate(SMapIterator* psIterator, void** ppvKey, size* piKeySize, void** ppvData, size* piDataSize)
{
	SMNode**	ppsNode;
	void*		pvKey;
	void*		pvData;
	SMNode*		psNode;

	ppsNode = (SMNode**)mapArray.Iterate(psIterator);
	if (!ppsNode)
	{
		return false;
	}

	psNode = *ppsNode;
	pvKey = RemapSinglePointer(psNode, sizeof(SMNode));
	pvData = RemapSinglePointer(pvKey, psNode->iKeySize);
	SafeAssign(ppvKey, pvKey);
	SafeAssign(ppvData, pvData);
	SafeAssign(piKeySize, psNode->iKeySize);
	SafeAssign(piDataSize, psNode->iValueSize);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapBlock::WriteExceptData(CFileWriter* pcFileWriter)
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
bool CMapBlock::ReadExceptData(CFileReader* pcFileReader, DataCompare fKeyCompare)
{
	CMallocator*	pcMalloc;
	bool			bResult;

	pcMalloc = gcMallocators.Read(pcFileReader);
	if (pcMalloc == NULL)
	{
		return false;
	}

	bResult = mapArray.ReadHeader(pcMalloc, pcFileReader, &CompareMNode);
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

	mfKeyCompare = fKeyCompare;
	miLargestKeySize = 0;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMNode* CMapBlock::WriteSizes(CFileWriter* pcFileWriter, size iIndex)
{
	SMNode*		psNode;

	psNode = *((SMNode**)mapArray.GetInSorted(iIndex));
	if (psNode)
	{
		if (!pcFileWriter->WriteSize(psNode->iKeySize))
		{
			return NULL;
		}
		if (!pcFileWriter->WriteSize(psNode->iValueSize))
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
SMNode* CMapBlock::ReadSizes(CFileReader* pcFileReader, size iIndex)
{
	SMNode*		psNode;
	SMNode**	ppsNode;
	size		iKeySize;
	size		iDataSize;

	if (!pcFileReader->ReadSize(&iKeySize))
	{
		return NULL;
	}
	if (!pcFileReader->ReadSize(&iDataSize))
	{
		return NULL;
	}
	psNode = AllocateNode(iKeySize, iDataSize);
	if (!psNode)
	{
		return NULL;
	}
	ppsNode = (SMNode**)mapArray.GetInSorted(iIndex);
	*ppsNode = psNode;

	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapBlock::Write(CFileWriter* pcFileWriter)
{
	size		iNumElements;
	size		i;
	bool		bResult;
	SMNode*		psNode;

	bResult = WriteExceptData(pcFileWriter);
	if (!bResult)
	{
		return false;
	}

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
bool CMapBlock::Read(CFileReader* pcFileReader)
{
	return Read(pcFileReader, &CompareMNodeKey, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapBlock::Read(CFileReader* pcFileReader, DataCompare fKeyCompare)
{
	return Read(pcFileReader, fKeyCompare, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapBlock::Read(CFileReader* pcFileReader, DataCompare fKeyCompare, CDataIO* pcDataIO)
{
	size		i;
	size		iNumElements;
	SMNode*		psNode;
	bool		bResult;

	if (fKeyCompare == NULL)
	{
		return false;
	}

	mpcDataIO = pcDataIO;
	if (!ReadExceptData(pcFileReader, fKeyCompare))
	{
		return false;
	}
	
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
bool CMapBlock::WriteData(CFileWriter* pcFileWriter, void* pvData)
{
	bool		bResult;
	SMNode*		psNode;
	void*		pvValue;
	void*		pvKey;
	
	psNode = (SMNode*)pvData;
	RemapKeyAndData(psNode, &pvKey, &pvValue);
	bResult = pcFileWriter->WriteData(pvKey, psNode->iKeySize);
	if (!bResult)
	{
		return false;
	}

	bResult = pcFileWriter->WriteData(pvValue, psNode->iValueSize);
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
bool CMapBlock::ReadData(CFileReader* pcFileReader, void* pvData)
{
	SMNode*		psNode;
	void*		pvKey;
	void*		pvValue;
	bool		bResult;

	psNode = (SMNode*)pvData;
	RemapKeyAndData(psNode, &pvKey, &pvValue);
	bResult = pcFileReader->ReadData(pvKey, psNode->iKeySize);
	if (!bResult)
	{
		return NULL;
	}

	bResult = pcFileReader->ReadData(pvValue, psNode->iValueSize);
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
SMNode* CMapBlock::AllocateNode(size iKeySize, size iDataSize)
{
	SMNode*	psNode;

	psNode = (SMNode*)mpcMalloc->Malloc(sizeof(SMNode) + iKeySize + iDataSize);
	if (!psNode)
	{
		return NULL;
	}

	psNode->iKeySize = iKeySize;
	psNode->iValueSize = iDataSize;
	psNode->fKeyCompare = mfKeyCompare;

	if (psNode->iKeySize > miLargestKeySize)
	{
		miLargestKeySize = psNode->iKeySize;
	}
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::RemapKeyAndData(SMNode* psNode, void** ppvKey, void** ppvData)
{
	*ppvKey = RemapSinglePointer(psNode, sizeof(SMNode));
	*ppvData = RemapSinglePointer(*ppvKey, psNode->iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBlockSorted* CMapBlock::GetArray(void)
{
	return &mapArray;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::SetDataFreeCallback(CDataFree* pcDataFree)
{
	mpcDataFree = pcDataFree;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::SetDataIOCallback(CDataIO* pcDataIO)
{
	mpcDataIO = pcDataIO;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::InsertHoldingIntoSorted(void)
{
	mapArray.InsertHoldingIntoSorted();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::GetInSorted(size iIndex, void** ppvKey, void** ppvData)
{
	SMNode**	ppsNode;
	void*		pvKey;
	void*		pvData;

	ppsNode = (SMNode**)mapArray.GetInSorted(iIndex);
	if (ppsNode)
	{
		pvKey = RemapSinglePointer(*ppsNode, sizeof(SMNode));
		pvData = RemapSinglePointer(pvKey, (*ppsNode)->iKeySize);
	}
	else
	{
		pvKey = NULL;
		pvData = NULL;
	}

	*ppvKey = pvKey;
	*ppvData = pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapBlock::GetValue(SMNode* psNode)
{
	return RemapSinglePointer(psNode, sizeof(SMNode) + psNode->iKeySize);

}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapBlock::GetKey(SMNode* psNode)
{
	return RemapSinglePointer(psNode, sizeof(SMNode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapBlock::ByteSize(void)
{
	SMapIterator	sIter;
	size				iKeySize;
	size				iDataSize;
	bool			bExists;
	size			uiByteSize;

	uiByteSize = 0;
	bExists = StartIteration(&sIter, NULL, &iKeySize, NULL, &iDataSize);
	while (bExists)
	{
		uiByteSize += sizeof(SMNode) + iKeySize + iDataSize;
		bExists = Iterate(&sIter, NULL, &iKeySize, NULL, &iDataSize);
	}
	return mapArray.ByteSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Pack(void)
{
	InsertHoldingIntoSorted();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Sort(void)
{
	mapArray.Sort();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Dump(void)
{
	mapArray.Dump();
}


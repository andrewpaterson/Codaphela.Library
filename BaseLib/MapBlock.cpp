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
void CMapBlock::Init(BOOL bOverwrite)
{
	Init(&gcSystemAllocator, NULL, &CompareMNodeKey, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(CMallocator* pcMalloc, BOOL bOverwrite)
{
	Init(pcMalloc, NULL, &CompareMNodeKey, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(int(*fKeyCompare)(const void*, const void*), BOOL bOverwrite)
{
	Init(&gcSystemAllocator, fKeyCompare, &CompareMNode, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(CMallocator* pcMalloc, int(*fKeyCompare)(const void*, const void*), BOOL bOverwrite)
{
	Init(pcMalloc, fKeyCompare, &CompareMNode, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(CMallocator* pcMalloc, int(*fKeyCompare)(const void*, const void*), CompareFunc fCompare, BOOL bOverwrite)
{
	int		iHoldingBufferSize;
	int		iHoldingBuffers;

	CMalloc::Init(pcMalloc);

	iHoldingBufferSize = 256;
	iHoldingBuffers = 4;

	this->fKeyCompare = fKeyCompare;
	mapArray.Init(pcMalloc, sizeof(void*), iHoldingBufferSize, iHoldingBuffers, fCompare);
	miLargestKeySize = 0;
	mbOverwrite = bOverwrite;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Kill(void)
{
	int			i;
	SMNode**	ppsNode;

	mapArray.InsertHoldingIntoSorted();
	for (i = 0; i < mapArray.NumElements(); i++)
	{
		ppsNode = (SMNode**)mapArray.GetInSorted(i);
		mpcMalloc->Free(*ppsNode);
	}

	miLargestKeySize = 0;
	mapArray.Kill();
	CMalloc::Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapBlock::Get(void* pvKey, int iKeySize)
{
	void*	pvData;
	BOOL	bFound;

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
BOOL CMapBlock::Get(void* pvKey, int iKeySize, void** ppvData, int* piDataSize)
{
	SMNode*		psNode;
	void*		pvData;

	psNode = GetNode(pvKey, iKeySize);
	if (!psNode)
	{
		return FALSE;
	}

	pvData = RemapSinglePointer(psNode, sizeof(SMNode) + psNode->iKeySize);

	SafeAssign(ppvData, pvData);
	SafeAssign(piDataSize, psNode->iDataSize);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapBlock::Put(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	void*		pvDestData;

	pvDestData = Put(pvKey, iKeySize, iDataSize);
	if (pvDestData != NULL)
	{
		if (pvData != NULL)
		{
			memcpy(pvDestData, pvData, iDataSize);
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapBlock::Put(void* pvKey, int iKeySize, int iDataSize)
{
	SMNode*			psNode;
	void*			pvDestKey;
	void*			pvDestData;
	BOOL			bResult;
	void*			pvExistingData;

	if (iKeySize > MAX_KEY_SIZE)
	{
		gcLogger.Error2(__METHOD__, " Key too long.  Cannot Put.", NULL);
		return NULL;
	}

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

	psNode = AllocateNode(iKeySize, iDataSize, &pvDestKey, &pvDestData);
	if (!psNode)
	{
		return NULL;
	}

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
BOOL CMapBlock::Remove(void* pvKey, int iKeySize)
{
	SMNode*		psNode;

	psNode = GetNode(pvKey, iKeySize);
	if (psNode)
	{
		mapArray.Remove(&psNode);
		mpcMalloc->Free(psNode);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CMapBlock::DataSize(void* pvKey, int iKeySize)
{
	SMNode* psNode;

	psNode = GetNode(pvKey, iKeySize);
	if (psNode)
	{
		return psNode->iDataSize;
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
BOOL CMapBlock::HasKey(void* pvKey, int iKeySize)
{
	SMNode* psNode;

	psNode = GetNode(pvKey, iKeySize);
	return psNode != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMNode* CMapBlock::GetNode(void* pvKey, int iKeySize)
{
	SMNode**	ppsNode;
	SMNode*		psSourceNode;
	void*		pvSourceKey;
	char		ac[MAX_KEY_SIZE + sizeof(SMNode)];

	if (iKeySize > MAX_KEY_SIZE)
	{
		gcLogger.Error2(__METHOD__, " Key too long.  Cannot Get.", NULL);
		return FALSE;
	}

	psSourceNode = (SMNode*)ac;

	psSourceNode->pcMapBlock = this;
	psSourceNode->iDataSize = 0;
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
int CMapBlock::NumElements(void)
{
	return mapArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapBlock::StartIteration(SMapIterator* psIterator, void** ppvKey, int* piKeySize, void** ppvData, int* piDataSize)
{
	SMNode**	ppsNode;
	void*		pvKey;
	void*		pvData;
	SMNode*		psNode;

	ppsNode = (SMNode**)mapArray.StartIteration(psIterator);
	if (!ppsNode)
	{
		return FALSE;
	}

	psNode = *ppsNode;
	pvKey = RemapSinglePointer(psNode, sizeof(SMNode));
	pvData = RemapSinglePointer(pvKey, psNode->iKeySize);
	SafeAssign(ppvKey, pvKey);
	SafeAssign(ppvData, pvData);
	SafeAssign(piKeySize, psNode->iKeySize);
	SafeAssign(piDataSize, psNode->iDataSize);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapBlock::Iterate(SMapIterator* psIterator, void** ppvKey, int* piKeySize, void** ppvData, int* piDataSize)
{
	SMNode**	ppsNode;
	void*		pvKey;
	void*		pvData;
	SMNode* psNode;

	ppsNode = (SMNode**)mapArray.Iterate(psIterator);
	if (!ppsNode)
	{
		return FALSE;
	}

	psNode = *ppsNode;
	pvKey = RemapSinglePointer(psNode, sizeof(SMNode));
	pvData = RemapSinglePointer(pvKey, psNode->iKeySize);
	SafeAssign(ppvKey, pvKey);
	SafeAssign(ppvData, pvData);
	SafeAssign(piKeySize, psNode->iKeySize);
	SafeAssign(piDataSize, psNode->iDataSize);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapBlock::WriteExceptData(CFileWriter* pcFileWriter)
{
	BOOL	bResult;

	InsertHoldingIntoSorted();

	bResult = gcMallocators.WriteMallocator(pcFileWriter, mpcMalloc);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = mapArray.WriteHeader(pcFileWriter);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = mapArray.GetSortedArray()->WriteHeader(pcFileWriter);
	if (!bResult)
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapBlock::ReadExceptData(CFileReader* pcFileReader, int(*fKeyCompare)(const void*, const void*))
{
	CMallocator*	pcMalloc;
	BOOL			bResult;

	pcMalloc = gcMallocators.ReadMallocator(pcFileReader);
	if (pcMalloc == NULL)
	{
		return FALSE;
	}

	bResult = mapArray.ReadHeader(pcMalloc, pcFileReader, &CompareMNode);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = mapArray.GetSortedArray()->ReadHeader(pcFileReader, pcMalloc);
	if (!bResult)
	{
		return FALSE;
	}

	CMalloc::Init(pcMalloc);

	this->fKeyCompare = fKeyCompare;
	miLargestKeySize = 0;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapBlock::WriteKey(CFileWriter* pcFileWriter, int iIndex, int* piDataSize)
{
	void*		pvKey;
	void*		pvData;
	SMNode*		psNode;

	psNode = *((SMNode**)mapArray.GetInSorted(iIndex));
	RemapKeyAndData(psNode, &pvKey, &pvData);

	if (!pcFileWriter->WriteInt(psNode->iKeySize))
	{
		return NULL;
	}
	if (!pcFileWriter->WriteInt(psNode->iDataSize))
	{
		return NULL;
	}
	if (!pcFileWriter->WriteData(pvKey, psNode->iKeySize))
	{
		return NULL;
	}

	*piDataSize = psNode->iDataSize;
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapBlock::ReadKey(CFileReader* pcFileReader, int iIndex, int* piDataSize)
{
	SMNode*		psNode;
	SMNode**	ppsNode;
	void*		pvKey;
	void*		pvData;
	int			iKeySize;
	int			iDataSize;

	if (!pcFileReader->ReadInt(&iKeySize))
	{
		return NULL;
	}
	if (!pcFileReader->ReadInt(&iDataSize))
	{
		return NULL;
	}
	psNode = AllocateNode(iKeySize, iDataSize, &pvKey, &pvData);
	if (!psNode)
	{
		return NULL;
	}
	ppsNode = (SMNode**)mapArray.GetInSorted(iIndex);
	*ppsNode = psNode;

	if (!pcFileReader->ReadData(pvKey, psNode->iKeySize))
	{
		return NULL;
	}

	*piDataSize = iDataSize;
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapBlock::Write(CFileWriter* pcFileWriter)
{
	int			iNumElements;
	int			i;
	void*		pvData;
	int			iDataSize;
	BOOL		bResult;


	bResult = WriteExceptData(pcFileWriter);
	if (!bResult)
	{
		return FALSE;
	}

	iNumElements = mapArray.GetSortedSize();
	for (i = 0; i < iNumElements; i++)
	{
		pvData = WriteKey(pcFileWriter, i, &iDataSize);
		if (!pvData)
		{
			return FALSE;
		}

		bResult = pcFileWriter->WriteData(pvData, iDataSize);
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapBlock::Read(CFileReader* pcFileReader, int(*fKeyCompare)(const void*, const void*))
{
	int			i;
	int			iNumElements;
	void*		pvData;
	int			iDataSize;

	if (fKeyCompare == NULL)
	{
		return FALSE;
	}

	if (!ReadExceptData(pcFileReader, fKeyCompare))
	{
		return FALSE;
	}
	
	iNumElements = mapArray.GetSortedSize();
	for (i = 0; i < iNumElements; i++)
	{
		pvData = ReadKey(pcFileReader, i, &iDataSize);
		if (!pvData)
		{
			return FALSE;
		}
		ReturnOnFalse(pcFileReader->ReadData(pvData, iDataSize));
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMNode* CMapBlock::AllocateNode(int iKeySize, int iDataSize, void** ppvKey, void** ppvData)
{
	SMNode*	psNode;

	psNode = (SMNode*)mpcMalloc->Malloc(sizeof(SMNode) + iKeySize + iDataSize);
	if (!psNode)
	{
		return NULL;
	}

	psNode->iKeySize = iKeySize;
	psNode->iDataSize = iDataSize;
	psNode->pcMapBlock = this;

	RemapKeyAndData(psNode, ppvKey, ppvData);
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
void CMapBlock::FinaliseSorted(void)
{
	InsertHoldingIntoSorted();
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
void CMapBlock::GetInSorted(int iIndex, void** ppvKey, void** ppvData)
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
size_t CMapBlock::ByteSize(void)
{
	SMapIterator	sIter;
	int				iKeySize;
	int				iDataSize;
	BOOL			bExists;
	size_t			uiByteSize;

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
void CMapBlock::Dump(void)
{
	mapArray.Dump();
}


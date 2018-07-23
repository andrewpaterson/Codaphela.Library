#include "DataMacro.h"
#include "GlobalMemory.h"
#include "MapBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareMNode(const void* arg1, const void* arg2)
{
	SMNode**	ppNode1;
	SMNode**	ppNode2;
	void*		pvKey1;
	void*		pvKey2;
	int			iResult;

	ppNode1 = (SMNode**)arg1;
	ppNode2 = (SMNode**)arg2;

	pvKey1 = HeaderGetData<SMNode, void>(*ppNode1);
	pvKey2 = HeaderGetData<SMNode, void>(*ppNode2);

	iResult = (*ppNode1)->pcMapBlock->fKeyCompare(pvKey1, pvKey2);
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(int(*fKeyCompare)(const void*, const void*), BOOL bOverwrite)
{
	Init(&gcSystemAllocator, fKeyCompare, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(CMallocator* pcMalloc, int(*fKeyCompare)(const void*, const void*), BOOL bOverwrite)
{
	int		iHoldingBufferSize;
	int		iHoldingBuffers;

	iHoldingBufferSize = 256;
	iHoldingBuffers = 4;

	mpcMalloc = pcMalloc;
	this->fKeyCompare = fKeyCompare;
	mapArray.Init(pcMalloc, sizeof(void*), iHoldingBufferSize, iHoldingBuffers, &CompareMNode);
	mapArray.SetOverwrite(bOverwrite);
	miLargestKeySize = 0;
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
	mpcMalloc = NULL;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMapBlock::Get(void* pvKey)
{
	void*	pvData;
	BOOL	bFound;

	bFound = Get(pvKey, &pvData, NULL);
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
BOOL CMapBlock::Get(void* pvKey, void** ppvData, int* piDataSize)
{
	SMNode**	ppsNode;
	void*		pvData;
	SMNode*		psSourceNode;
	void*		pvSourceKey;
	int			iKeySize;
	char		ac[1024];

	if (pvKey == NULL)
	{
		return FALSE;
	}

	if (miLargestKeySize + sizeof(SMNode) >= 1024)
	{
		return FALSE;
	}

	psSourceNode = (SMNode*)ac;

	psSourceNode->pcMapBlock = this;
	psSourceNode->iDataSize = 0;
	psSourceNode->iKeySize = 0;
	pvSourceKey = RemapSinglePointer(psSourceNode, sizeof(SMNode));
	memcpy(pvSourceKey, pvKey, miLargestKeySize);

	ppsNode = (SMNode**)mapArray.Get(&psSourceNode);

	if (!ppsNode)
	{
		*ppvData = NULL;
		return FALSE;
	}

	iKeySize = (*ppsNode)->iKeySize;
	pvData = RemapSinglePointer((*ppsNode), sizeof(SMNode) + iKeySize);

	*ppvData = pvData;
	if (piDataSize != NULL)
	{
		*piDataSize = (*ppsNode)->iDataSize;
	}

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
	SMNode*		psNode;
	void*		pvDestKey;
	void*		pvDestData;
	BOOL		bResult;

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
BOOL CMapBlock::Remove(void* pvKey)
{
	SMNode**	ppsNode;
	SMNode*		psSourceNode;
	void*		pvSourceKey;
	char		ac[1024];
	SMNode*		psNode;

	if (miLargestKeySize + sizeof(SMNode) >= 1024)
	{
		return FALSE;
	}

	psSourceNode = (SMNode*)ac;

	psSourceNode->pcMapBlock = this;
	psSourceNode->iDataSize = 0;
	psSourceNode->iKeySize = 0;
	pvSourceKey = RemapSinglePointer(ac, sizeof(SMNode));
	memcpy(pvSourceKey, pvKey, miLargestKeySize);

	ppsNode = (SMNode**)mapArray.Get(&psSourceNode);

	if (!ppsNode)
	{
		return FALSE;
	}

	psNode = *ppsNode;
	mapArray.Remove(&psNode);

	mpcMalloc->Free(psNode);

	return TRUE;
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
BOOL CMapBlock::StartIteration(SMapIterator* psIterator, void** ppvKey, void** ppvData)
{
	SMNode**	ppsNode;
	void*		pvKey;
	void*		pvData;

	ppsNode = (SMNode**)mapArray.StartIteration(psIterator);
	if (!ppsNode)
	{
		return FALSE;
	}

	pvKey = RemapSinglePointer(*ppsNode, sizeof(SMNode));
	pvData = RemapSinglePointer(pvKey, (*ppsNode)->iKeySize);
	if (ppvKey)
	{
		*ppvKey = pvKey;
	}
	if (ppvData)
	{
		*ppvData = pvData;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapBlock::Iterate(SMapIterator* psIterator, void** ppvKey, void** ppvData)
{
	SMNode**	ppsNode;
	void*		pvKey;
	void*		pvData;

	ppsNode = (SMNode**)mapArray.Iterate(psIterator);
	if (!ppsNode)
	{
		return FALSE;
	}


	pvKey = RemapSinglePointer(*ppsNode, sizeof(SMNode));
	pvData = RemapSinglePointer(pvKey, (*ppsNode)->iKeySize);
	if (ppvKey)
	{
		*ppvKey = pvKey;
	}
	if (ppvData)
	{
		*ppvData = pvData;
	}
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

	mpcMalloc = pcMalloc;
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

	psNode = AllocateNode(iKeySize, iDataSize);
	if (!psNode)
	{
		return NULL;
	}
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
SMNode* CMapBlock::AllocateNode(int iKeySize, int iDataSize)
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


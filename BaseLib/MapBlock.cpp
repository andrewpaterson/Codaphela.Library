#include "DataMacro.h"
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

	iResult = (*ppNode1)->pcMapBlock->Func(pvKey1, pvKey2);
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Init(CMallocator* pcMalloc, int iChunkSize, int(* Func)(const void*, const void*))
{
	int		iHoldingBufferSize;
	int		iHoldingBuffers;

	if (iChunkSize >= 256)
	{
		iHoldingBufferSize = 256;
		iHoldingBuffers = 4;
	}
	if (iChunkSize >= 128)
	{
		iHoldingBufferSize = 128;
		iHoldingBuffers = 2;
	}
	else
	{
		iHoldingBuffers = 1;
		iHoldingBufferSize = 32;
	}

	mpcMalloc = pcMalloc;
	this->Func = Func;
	mapArray.Init(pcMalloc, sizeof(void*), iChunkSize, iHoldingBufferSize, iHoldingBuffers, &CompareMNode);
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
BOOL CMapBlock::Get(void* pvKey, void** ppvData, int* piDataSize)
{
	SMNode**	ppsNode;
	void*		pvData;
	SMNode*		psSourceNode;
	void*		pvSourceKey;
	int			iKeySize;
	char		ac[1024];

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

	psNode = (SMNode*)mpcMalloc->Malloc(sizeof(SMNode) + iKeySize + iDataSize);
	pvDestKey = RemapSinglePointer(psNode, sizeof(SMNode));
	pvDestData = RemapSinglePointer(pvDestKey, iKeySize);

	if (psNode == NULL)
	{
		return NULL;
	}

	psNode->iDataSize = iDataSize;
	psNode->iKeySize = iKeySize;
	psNode->pcMapBlock = this;

	memcpy(pvDestKey, pvKey, iKeySize);

	if (miLargestKeySize < iKeySize)
	{
		miLargestKeySize = iKeySize;
	}

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
BOOL CMapBlock::Write(CFileWriter* pcFileWriter)
{
	int			iNumElements;
	int			i;
	SMNode*		psNode;
	void*		pvKey;
	void*		pvData;

	InsertHoldingIntoSorted();

	if (!mapArray.WriteHeader(pcFileWriter))
	{
		return FALSE;
	}

	if (!mapArray.GetSortedArray()->WriteHeader(pcFileWriter))
	{
		return FALSE;
	}

	iNumElements = mapArray.GetSortedSize();
	for (i = 0; i < iNumElements; i++)
	{
		psNode = *((SMNode**)mapArray.GetInSorted(i));
		pvKey = RemapSinglePointer(psNode, sizeof(SMNode));
		pvData = RemapSinglePointer(pvKey, psNode->iKeySize);

		ReturnOnFalse(pcFileWriter->WriteInt(psNode->iKeySize));
		ReturnOnFalse(pcFileWriter->WriteInt(psNode->iDataSize));
		ReturnOnFalse(pcFileWriter->WriteData(pvKey, psNode->iKeySize));
		ReturnOnFalse(pcFileWriter->WriteData(pvData, psNode->iDataSize));
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapBlock::Read(CFileReader* pcFileReader, int(*Func)(const void*, const void*))
{
	int			i;
	SMNode*		psNode;
	SMNode**	ppsNode;
	int			iNumElements;
	void*		pvKey;
	void*		pvData;
	int			iKeySize;
	int			iDataSize;

	if (!mapArray.ReadHeader(pcFileReader, &CompareMNode))
	{
		return FALSE;
	}

	if (!mapArray.GetSortedArray()->ReadHeader(pcFileReader))
	{
		return FALSE;
	}

	mpcMalloc = &gcSystemAllocator;
	this->Func = Func;
	
	iNumElements = mapArray.GetSortedSize();
	miLargestKeySize = 0;
	for (i = 0; i < iNumElements; i++)
	{
		ReturnOnFalse(pcFileReader->ReadInt(&iKeySize));
		ReturnOnFalse(pcFileReader->ReadInt(&iDataSize));

		psNode = (SMNode*)mpcMalloc->Malloc(sizeof(SMNode) + iKeySize + iDataSize);
		psNode->iKeySize = iKeySize;
		psNode->iDataSize = iDataSize;
		psNode->pcMapBlock = this;

		pvKey = RemapSinglePointer(psNode, sizeof(SMNode));
		pvData = RemapSinglePointer(pvKey, psNode->iKeySize);

		ReturnOnFalse(pcFileReader->ReadData(pvKey, psNode->iKeySize));
		ReturnOnFalse(pcFileReader->ReadData(pvData, psNode->iDataSize));

		if (psNode->iKeySize > miLargestKeySize)
		{
			miLargestKeySize = psNode->iKeySize;
		}

		ppsNode = (SMNode**)mapArray.GetInSorted(i);
		*ppsNode = psNode;
	}
	return TRUE;
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


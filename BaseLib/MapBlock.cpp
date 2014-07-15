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
	mpcMalloc = pcMalloc;
	this->Func = Func;
	mapArray.Init(pcMalloc, sizeof(void*), iChunkSize, 128, 2, &CompareMNode);
	miLargestKeySize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapBlock::Kill(void)
{
	int			i;
	SMNode*		psNode;

	mapArray.InsertHoldingIntoSorted();
	for (i = 0; i < mapArray.NumElements(); i++)
	{
		psNode = (SMNode*)mapArray.GetInSorted(i);
		mpcMalloc->Free(psNode);
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
	SMNode*		psNode;
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

	psNode = (SMNode*)mapArray.Get(&psSourceNode);

	if (!psNode)
	{
		*ppvData = NULL;
		return FALSE;
	}

	iKeySize = psNode->iKeySize;
	pvData = RemapSinglePointer(psNode, sizeof(SMNode) + iKeySize);

	*ppvData = pvData;
	if (piDataSize != NULL)
	{
		*piDataSize = psNode->iDataSize;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapBlock::Put(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	SMNode*		psNode;
	void*		pvDestKey;
	void*		pvDestData;
	BOOL		bResult;
	
	psNode = (SMNode*)mpcMalloc->Malloc(sizeof(SMNode) + iKeySize + iDataSize);
	pvDestKey = RemapSinglePointer(psNode, sizeof(SMNode));
	pvDestData = RemapSinglePointer(pvDestKey, iKeySize);

	psNode->iDataSize = iDataSize;
	psNode->iKeySize = iKeySize;
	psNode->pcMapBlock = this;

	memcpy(pvDestKey, pvKey, iKeySize);
	memcpy(pvDestData, pvData, iDataSize);

	if (miLargestKeySize < iKeySize)
	{
		miLargestKeySize = iKeySize;
	}

	bResult = mapArray.Add(&psNode);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMapBlock::NumElements(void)
{
	return mapArray.NumElements();
}


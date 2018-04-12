#include "IndexTreeMemoryAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemoryAccess::Init(CIndexTreeMemory* pcTree)
{
	CIndexTreeAccess::Init();
	mpcTree = pcTree;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeMemoryAccess::Kill(void)
{
	mpcTree = NULL;
	CIndexTreeAccess::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemoryAccess::Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiDataSize)
{
	void* pvResult;

	pvResult = mpcTree->Put(pvKey, iKeySize, pvObject, uiDataSize);
	return pvResult != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeMemoryAccess::Get(void* pvKey, int iKeySize, void* pvObject, int* piDataSize)
{
	void*			pvResult;
	unsigned short	uiDataSize;

	pvResult = mpcTree->Get(pvKey, iKeySize, &uiDataSize);
	
	if (piDataSize)
	{
		*piDataSize = uiDataSize;
	}

	if (pvResult)
	{
		if (pvObject)
		{
			memcpy(pvObject, pvResult, uiDataSize);
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
BOOL CIndexTreeMemoryAccess::Remove(void* pvKey, int iKeySize)
{
	return mpcTree->Remove(pvKey, iKeySize);
}

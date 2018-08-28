#include "IndexTreeEvictingAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Init(CIndexTreeEvicting* pcTree)
{
	CIndexTreeAccess::Init();
	mpcTree = pcTree;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Kill(void)
{
	mpcTree = NULL;
	return CIndexTreeAccess::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Flush(void)
{
	if (!mpcTree->IsWriteThrough())
	{
		return mpcTree->Flush();
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiDataSize)
{
	return mpcTree->Put(pvKey, iKeySize, pvObject, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Get(void* pvKey, int iKeySize, void* pvObject, int* piDataSize)
{
	unsigned short	uiDataSize;
	BOOL			bResult;

	bResult = mpcTree->Get(pvKey, iKeySize, pvObject, &uiDataSize);

	if (piDataSize)
	{
		*piDataSize = uiDataSize;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeEvictingAccess::Remove(void* pvKey, int iKeySize)
{
	return mpcTree->Remove(pvKey, iKeySize);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short CIndexTreeEvictingAccess::DataSize(void* pvKey, int iKeySize)
{
	return mpcTree->ObjectSize(pvKey, iKeySize);
}

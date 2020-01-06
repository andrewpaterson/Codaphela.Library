#include "EvictionCallbackWrapper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEvictionCallbackWrapper::Init(CIndexTreeEvictionCallback* pcCallback1, CIndexTreeEvictionCallback* pcCallback2)
{
	mpcCallback1 = pcCallback1;
	mpcCallback2 = pcCallback2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CEvictionCallbackWrapper::IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	BOOL	bResult;
	
	bResult = mpcCallback1->IndexTreeNodeEvicted(pvKey, iKeySize, pvData, iDataSize);
	bResult &= mpcCallback2->IndexTreeNodeEvicted(pvKey, iKeySize, pvData, iDataSize);
	return bResult;
}


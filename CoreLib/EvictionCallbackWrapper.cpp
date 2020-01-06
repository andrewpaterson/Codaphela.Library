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
BOOL CEvictionCallbackWrapper::NodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	BOOL	bResult;
	
	bResult = mpcCallback1->NodeEvicted(pvKey, iKeySize, pvData, iDataSize);
	bResult &= mpcCallback2->NodeEvicted(pvKey, iKeySize, pvData, iDataSize);
	return bResult;
}


#include "IndexTreeEvictionCallbackWrapper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeEvictionCallbackWrapper::Init(CIndexTreeEvictionCallback* pcCallback1, CIndexTreeEvictionCallback* pcCallback2)
{
	mpcCallback1 = pcCallback1;
	mpcCallback2 = pcCallback2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeEvictionCallbackWrapper::IndexTreeNodeEvicted(void* pvKey, size iKeySize, void* pvData, size iDataSize)
{
	bool	bResult;
	
	bResult = mpcCallback1->IndexTreeNodeEvicted(pvKey, iKeySize, pvData, iDataSize);
	bResult &= mpcCallback2->IndexTreeNodeEvicted(pvKey, iKeySize, pvData, iDataSize);
	return bResult;
}


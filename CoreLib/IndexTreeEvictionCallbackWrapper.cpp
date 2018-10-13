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
BOOL CIndexTreeEvictionCallbackWrapper::NodeEvicted(CIndexTreeFile* pcIndexTree, unsigned char* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	BOOL	bResult;
	
	bResult = mpcCallback1->NodeEvicted(pcIndexTree, pvKey, iKeySize, pvData, iDataSize);
	bResult &= mpcCallback2->NodeEvicted(pcIndexTree, pvKey, iKeySize, pvData, iDataSize);
	return bResult;
}


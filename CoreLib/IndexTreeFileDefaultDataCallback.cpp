#include "BaseLib/FastMemcpy.h"
#include "IndexTreeFileDefaultDataCallback.h"


CIndexTreeFileDefaultDataCallback gcIndexTreeFileDefaultCallback;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short CIndexTreeFileDefaultDataCallback::IndexTreeDataSize(unsigned short uiSourceSize)
{
	return uiSourceSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFileDefaultDataCallback::IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, unsigned short uiSourceDataSize)
{
	if (iFileDataSize == uiSourceDataSize)
	{
		memcpy_fast(pvDataBuffer, pvSource, uiSourceDataSize);
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
BOOL CIndexTreeFileDefaultDataCallback::IndexTreeReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestDataSize, int iFileDataSize)
{
	if (uiDestDataSize == iFileDataSize)
	{
		memcpy_fast(pvDest, pvDataBuffer, uiDestDataSize);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


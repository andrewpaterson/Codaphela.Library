#include "BaseLib/FastMemcpy.h"
#include "IndexTreeFileDefaultDataCallback.h"


CIndexTreeFileDefaultDataCallback gcIndexTreeFileDefaultCallback;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CIndexTreeFileDefaultDataCallback::IndexTreeDataSize(uint16 uiSourceSize)
{
	return uiSourceSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFileDefaultDataCallback::IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, uint16 uiSourceDataSize)
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
BOOL CIndexTreeFileDefaultDataCallback::IndexTreeReadData(void* pvDest, void* pvDataBuffer, uint16 uiDestDataSize, int iFileDataSize)
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


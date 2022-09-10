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
bool CIndexTreeFileDefaultDataCallback::IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, uint16 uiSourceDataSize)
{
	if (iFileDataSize == uiSourceDataSize)
	{
		memcpy_fast(pvDataBuffer, pvSource, uiSourceDataSize);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileDefaultDataCallback::IndexTreeReadData(void* pvDest, void* pvDataBuffer, uint16 uiDestDataSize, int iFileDataSize)
{
	if (uiDestDataSize == iFileDataSize)
	{
		memcpy_fast(pvDest, pvDataBuffer, uiDestDataSize);
		return true;
	}
	else
	{
		return false;
	}
}


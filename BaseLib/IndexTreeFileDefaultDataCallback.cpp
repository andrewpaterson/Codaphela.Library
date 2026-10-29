#include <string.h>
#include "IndexTreeFileDefaultDataCallback.h"


CIndexTreeFileDefaultDataCallback gcIndexTreeFileDefaultCallback;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeFileDefaultDataCallback::IndexTreeDataSize(size uiSourceSize)
{
	return uiSourceSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeFileDefaultDataCallback::IndexTreeWriteData(void* pvDataBuffer, void* pvSource, size iFileDataSize, size uiSourceDataSize)
{
	if (iFileDataSize == uiSourceDataSize)
	{
		memcpy(pvDataBuffer, pvSource, uiSourceDataSize);
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
bool CIndexTreeFileDefaultDataCallback::IndexTreeReadData(void* pvDest, void* pvDataBuffer, size uiDestDataSize, size iFileDataSize)
{
	if (uiDestDataSize == iFileDataSize)
	{
		memcpy(pvDest, pvDataBuffer, uiDestDataSize);
		return true;
	}
	else
	{
		return false;
	}
}


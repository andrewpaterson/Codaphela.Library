#include "BaseLib/FastMemcpy.h"
#include "IndexTreeFileDefaultCallback.h"


CIndexTreeFileDefaultCallback gcIndexTreeFileDefaultCallback;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short CIndexTreeFileDefaultCallback::DataBufferSize(unsigned short uiSourceSize)
{
	return uiSourceSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFileDefaultCallback::WriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, unsigned short uiSourceDataSize)
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
BOOL CIndexTreeFileDefaultCallback::ReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestDataSize, int iFileDataSize)
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


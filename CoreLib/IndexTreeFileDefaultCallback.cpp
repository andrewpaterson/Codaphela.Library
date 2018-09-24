#include "IndexTreeFileDefaultCallback.h"
#include "BaseLib/FastMemcpy.h"


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
BOOL CIndexTreeFileDefaultCallback::WriteData(void* pvDataBuffer, void* pvSource, int iFileSize, unsigned short uiSourceSize)
{
		memcpy_fast(pvDataBuffer, pvSource, uiSourceSize);
		return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexTreeFileDefaultCallback::ReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestSize, int iFileSize)
{
		memcpy_fast(pvDest, pvDataBuffer, uiDestSize);
		return TRUE;
}


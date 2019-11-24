#include "BaseLib/FastMemcpy.h"
#include "IndexedDataDescriptor.h"
#include "IndexedDescriptorsFileCallback.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short CIndexedDescriptorsFileCallback::DataBufferSize(unsigned short uiSourceSize)
{
	if (uiSourceSize != 0)
	{
		return sizeof(SIndexedFileDataDescriptor);
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFileCallback::WriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, unsigned short uiSourceDataSize)
{
	memcpy_fast(pvDataBuffer, pvSource, iFileDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFileCallback::ReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestDataSize, int iFileDataSize)
{
	memset_fast(pvDest, 0, uiDestDataSize);
	memcpy_fast(pvDest, pvDataBuffer, iFileDataSize);
	return TRUE;
}


#ifndef __INDEXED_DESCRIPTORS_FILE_CALLBACK_H__
#define __INDEXED_DESCRIPTORS_FILE_CALLBACK_H__
#include "IndexTreeFileCallback.h"


class CIndexedDescriptorsFileCallback : public CIndexTreeFileCallback
{
public:
	unsigned short	DataBufferSize(unsigned short uiSourceSize);
	BOOL			WriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, unsigned short uiSourceDataSize);
	BOOL			ReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestDataSize, int iFileDataSize);
};


#endif // __INDEXED_DESCRIPTORS_FILE_CALLBACK_H__


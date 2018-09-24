#ifndef __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__
#define __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__
#include "IndexTreeFileCallback.h"


class CIndexTreeFileDefaultCallback : public CIndexTreeFileCallback
{
public:
	unsigned short	DataBufferSize(unsigned short uiSourceSize);
	BOOL			WriteData(void* pvDataBuffer, void* pvSource, int iFileSize, unsigned short uiSourceSize);
	BOOL			ReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestSize, int iFileSize);
};


extern CIndexTreeFileDefaultCallback gcIndexTreeFileDefaultCallback;


#endif // __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__


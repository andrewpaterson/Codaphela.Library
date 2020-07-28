#ifndef __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__
#define __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__
#include "IndexTreeFileDataCallback.h"


class CIndexTreeFileDefaultDataCallback : public CIndexTreeFileDataCallback
{
public:
	unsigned short	IndexTreeDataSize(unsigned short uiSourceSize);
	BOOL			IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, unsigned short uiSourceDataSize);
	BOOL			IndexTreeReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestDataSize, int iFileDataSize);
};


extern CIndexTreeFileDefaultDataCallback gcIndexTreeFileDefaultCallback;


#endif // __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__


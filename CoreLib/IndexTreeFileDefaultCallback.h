#ifndef __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__
#define __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__
#include "IndexTreeFileCallback.h"


class CIndexTreeFileDefaultCallback : public CIndexTreeFileCallback
{
public:
	unsigned short	IndexTreeDataSize(unsigned short uiSourceSize);
	BOOL			IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, unsigned short uiSourceDataSize);
	BOOL			IndexTreeReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestDataSize, int iFileDataSize);
};


extern CIndexTreeFileDefaultCallback gcIndexTreeFileDefaultCallback;


#endif // __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__


#ifndef __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__
#define __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__
#include "IndexTreeFileDataCallback.h"


class CIndexTreeFileDefaultDataCallback : public CIndexTreeFileDataCallback
{
public:
	size	IndexTreeDataSize(size uiSourceSize);
	bool	IndexTreeWriteData(void* pvDataBuffer, void* pvSource, size iFileDataSize, size uiSourceDataSize);
	bool	IndexTreeReadData(void* pvDest, void* pvDataBuffer, size uiDestDataSize, size iFileDataSize);
};


extern CIndexTreeFileDefaultDataCallback gcIndexTreeFileDefaultCallback;


#endif // __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__


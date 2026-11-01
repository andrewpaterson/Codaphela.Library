#ifndef __INDEX_TREE_FILE_TEST_CALLBACK_H__
#define __INDEX_TREE_FILE_TEST_CALLBACK_H__
#include "BaseLib/IndexTreeFileDataCallback.h"


class CIndexTreeFileDataTestCallback : public CIndexTreeFileDataCallback
{
public:
	size			IndexTreeDataSize(size uiSourceSize);
	bool			IndexTreeWriteData(void* pvDataBuffer, void* pvSource, size iFileDataSize, size uiSourceDataSize);
	bool			IndexTreeReadData(void* pvDest, void* pvDataBuffer, size uiDestDataSize, size iFileDataSize);
};


#endif // __INDEX_TREE_FILE_TEST_CALLBACK_H__


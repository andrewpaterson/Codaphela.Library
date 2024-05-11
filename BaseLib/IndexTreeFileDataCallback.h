#ifndef __INDEX_TREE_FILE_CALLBACK_H__
#define __INDEX_TREE_FILE_CALLBACK_H__
#include "PrimitiveTypes.h"


class CIndexTreeFileDataCallback
{
public:
	virtual size		IndexTreeDataSize(size uiSourceSize) =0;
	virtual bool		IndexTreeWriteData(void* pvDataBuffer, void* pvSource, size iFileSize, size uiSourceSize) =0;
	virtual bool		IndexTreeReadData(void* pvDest, void* pvDataBuffer, size uiDestSize, size iFileSize) =0;
};


#endif // __INDEX_TREE_FILE_CALLBACK_H__



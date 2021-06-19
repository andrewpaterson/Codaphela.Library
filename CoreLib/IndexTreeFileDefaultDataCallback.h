#ifndef __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__
#define __INDEX_TREE_FILE_DEFAULT_CALLBACK_H__
#include "IndexTreeFileDataCallback.h"


class CIndexTreeFileDefaultDataCallback : public CIndexTreeFileDataCallback
{
public:
	uint16	IndexTreeDataSize(uint16 uiSourceSize);
	BOOL			IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, uint16 uiSourceDataSize);
	BOOL			IndexTreeReadData(void* pvDest, void* pvDataBuffer, uint16 uiDestDataSize, int iFileDataSize);
};


extern CIndexTreeFileDefaultDataCallback gcIndexTreeFileDefaultCallback;


#endif // !__INDEX_TREE_FILE_DEFAULT_CALLBACK_H__


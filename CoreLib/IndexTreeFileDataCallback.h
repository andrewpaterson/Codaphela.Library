#ifndef __INDEX_TREE_FILE_CALLBACK_H__
#define __INDEX_TREE_FILE_CALLBACK_H__
#include "BaseLib/PrimitiveTypes.h"


class CIndexTreeFileDataCallback
{
public:
	virtual uint16		IndexTreeDataSize(uint16 uiSourceSize) =0;
	virtual BOOL		IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileSize, uint16 uiSourceSize) =0;
	virtual BOOL		IndexTreeReadData(void* pvDest, void* pvDataBuffer, uint16 uiDestSize, int iFileSize) =0;
};


#endif // __INDEX_TREE_FILE_CALLBACK_H__



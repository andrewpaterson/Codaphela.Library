#ifndef __INDEX_TREE_FILE_CALLBACK_H__
#define __INDEX_TREE_FILE_CALLBACK_H__
#include "PrimitiveTypes.h"


class CIndexTreeFileDataCallback
{
public:
	virtual uint16		IndexTreeDataSize(uint16 uiSourceSize) =0;
	virtual bool		IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileSize, uint16 uiSourceSize) =0;
	virtual bool		IndexTreeReadData(void* pvDest, void* pvDataBuffer, uint16 uiDestSize, int iFileSize) =0;
};


#endif // !__INDEX_TREE_FILE_CALLBACK_H__



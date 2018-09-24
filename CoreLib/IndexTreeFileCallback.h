#ifndef __INDEX_TREE_FILE_CALLBACK_H__
#define __INDEX_TREE_FILE_CALLBACK_H__
#include "BaseLib/Bool.h"


class CIndexTreeFileCallback
{
public:
	virtual unsigned short	DataBufferSize(unsigned short uiSourceSize) = 0;
	virtual BOOL			WriteData(void* pvDataBuffer, void* pvSource, int iFileSize, unsigned short uiSourceSize) =0;
	virtual BOOL			ReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestSize, int iFileSize) =0;
};


#endif // __INDEX_TREE_FILE_CALLBACK_H__



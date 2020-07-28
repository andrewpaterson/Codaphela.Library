#ifndef __INDEX_TREE_FILE_CALLBACK_H__
#define __INDEX_TREE_FILE_CALLBACK_H__
#include "BaseLib/Bool.h"


//xxx //Rename this to CIndexTreeFileDataCallback
class CIndexTreeFileCallback
{
public:
	virtual unsigned short	IndexTreeDataSize(unsigned short uiSourceSize) =0;
	virtual BOOL			IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileSize, unsigned short uiSourceSize) =0;
	virtual BOOL			IndexTreeReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestSize, int iFileSize) =0;
};


#endif // __INDEX_TREE_FILE_CALLBACK_H__



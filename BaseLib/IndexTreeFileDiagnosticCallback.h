#ifndef __INDEX_TREE_FILE_DIAGNOSTIC_CALLBACK_H__
#define __INDEX_TREE_FILE_DIAGNOSTIC_CALLBACK_H__
#include "Define.h"


class CIndexTreeFileDiagnosticCallback
{
public:
			void	Init(void);
	virtual void	Kill(void);

	virtual void	Put(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	virtual void	Get(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	virtual void	Remove(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	virtual void	Flush(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	virtual void	Evict(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	//virtual void	Read(void* pvKey, int iKeySize, void* pvData, int iDataSize, void* pvBuffer, int iBufferSize);
	//virtual void	Write(void* pvKey, int iKeySize, void* pvData, int iDataSize, void* pvBuffer, int iBufferSize);
};


#endif // __INDEX_TREE_FILE_DIAGNOSTIC_CALLBACK_H__


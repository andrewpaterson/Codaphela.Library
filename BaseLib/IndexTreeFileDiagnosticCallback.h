#ifndef __INDEX_TREE_FILE_DIAGNOSTIC_CALLBACK_H__
#define __INDEX_TREE_FILE_DIAGNOSTIC_CALLBACK_H__
#include "Define.h"
#include "PrimitiveTypes.h"


class CIndexTreeFileDiagnosticCallback
{
public:
			void	Init(void);
	virtual void	Kill(void);

	virtual void	Put(void* pvKey, size iKeySize, void* pvData, size iDataSize);
	virtual void	Get(void* pvKey, size iKeySize, void* pvData, size iDataSize);
	virtual void	Remove(void* pvKey, size iKeySize, void* pvData, size iDataSize);
	virtual void	Flush(void* pvKey, size iKeySize, void* pvData, size iDataSize);
	virtual void	Evict(void* pvKey, size iKeySize, void* pvData, size iDataSize);
	//virtual void	Read(void* pvKey, size iKeySize, void* pvData, size iDataSize, void* pvBuffer, size iBufferSize);
	//virtual void	Write(void* pvKey, size iKeySize, void* pvData, size iDataSize, void* pvBuffer, size iBufferSize);
};


#endif // __INDEX_TREE_FILE_DIAGNOSTIC_CALLBACK_H__


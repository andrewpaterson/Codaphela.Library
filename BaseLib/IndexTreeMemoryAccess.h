#ifndef __INDEX_TREE_MEMORY_ACCESS_H__
#define __INDEX_TREE_MEMORY_ACCESS_H__
#include "IndexTreeMemory.h"
#include "IndexTreeAccess.h"


class CIndexTreeMemoryAccess : public CIndexTreeAccess
{
private:
	CIndexTreeMemory*	mpcTree;

public:
	BOOL	Init(CIndexTreeMemory* pcTree);
	BOOL	Kill(void);
	BOOL	Flush(void);
	int64	NumElements(void);
	BOOL	ValidateIndex(void);

protected:
	BOOL	Put(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	BOOL	Get(void* pvKey, int iKeySize, void* pvData, int* piDataSize);
	BOOL	Remove(void* pvKey, int iKeySize);
	BOOL	Has(void* pvKey, int iKeySize);
	int		DataSize(void* pvKey, int iKeySize);
	BOOL	Flush(void* pvKey, int iKeySize);
	BOOL	Evict(void* pvKey, int iKeySize);
};


#endif // __INDEX_TREE_MEMORY_ACCESS_H__


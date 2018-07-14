#ifndef __INDEX_TREE_MEMORY_ACCESS_H__
#define __INDEX_TREE_MEMORY_ACCESS_H__
#include "BaseLib/IndexTreeMemory.h"
#include "IndexTreeAccess.h"


class CIndexTreeMemoryAccess : public CIndexTreeAccess
{
private:
	CIndexTreeMemory*	mpcTree;

public:
	BOOL	Init(CIndexTreeMemory* pcTree);
	BOOL	Kill(void);
	BOOL	Flush(void);

protected:
	BOOL			Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiDataSize);
	BOOL			Get(void* pvKey, int iKeySize, void* pvObject, int* piDataSize);
	BOOL			Remove(void* pvKey, int iKeySize);
	unsigned short	DataSize(void* pvKey, int iKeySize);
};


#endif // __INDEX_TREE_MEMORY_ACCESS_H__



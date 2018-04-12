#ifndef __INDEX_TREE_MEMORY_ACCESS_H__
#define __INDEX_TREE_MEMORY_ACCESS_H__
#include "BaseLib/IndexTreeMemory.h"
#include "IndexTreeAccess.h"


class CIndexTreeMemoryAccess : public CIndexTreeAccess
{
private:
	CIndexTreeMemory*	mpcTree;

public:
	void	Init(CIndexTreeMemory* pcTree);
	void	Kill(void);

protected:
	BOOL	Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiDataSize);
	BOOL	Get(void* pvKey, int iKeySize, void* pvObject, int* piDataSize);
	BOOL	Remove(void* pvKey, int iKeySize);
};


#endif // __INDEX_TREE_MEMORY_ACCESS_H__


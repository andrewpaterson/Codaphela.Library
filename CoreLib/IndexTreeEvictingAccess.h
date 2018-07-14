#ifndef __INDEX_TREE_EVICTING_ACCESS_H__
#define __INDEX_TREE_EVICTING_ACCESS_H__
#include "IndexTreeEvicting.h"
#include "IndexTreeAccess.h"


class CIndexTreeEvictingAccess : public CIndexTreeAccess
{
private:
	CIndexTreeEvicting*		mpcTree;

public:
	BOOL	Init(CIndexTreeEvicting* pcTree);
	BOOL	Kill(void);
	BOOL	Flush(void);

protected:
	BOOL	Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiDataSize);
	BOOL	Get(void* pvKey, int iKeySize, void* pvObject, int* piDataSize);
	BOOL	Remove(void* pvKey, int iKeySize);
	unsigned short	DataSize(void* pvKey, int iKeySize);
};



#endif // __INDEX_TREE_EVICTING_ACCESS_H__


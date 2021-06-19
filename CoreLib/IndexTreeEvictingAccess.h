#ifndef __INDEX_TREE_EVICTING_ACCESS_H__
#define __INDEX_TREE_EVICTING_ACCESS_H__
#include "BaseLib/IndexAccess.h"
#include "IndexTreeEvicting.h"


class CIndexTreeEvictingAccess : public CIndexAccess
{
private:
	CIndexTreeEvicting*		mpcTree;

public:
	BOOL				Init(CIndexTreeEvicting* pcTree);
	BOOL				Kill(void);
	BOOL				Flush(void);
	int64				NumElements(void);
	BOOL				ValidateIndex(void);
	void				Dump(void);

	CIndexTreeIterator* CreateIterator(void);
	void				FreeIterator(CIndexTreeIterator* pcIter);

protected:
	BOOL	Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
	BOOL	Get(void* pvKey, int iKeySize, void* pvData, size_t* uiDataSize, size_t uiMaxDataSize);
	BOOL	Remove(void* pvKey, int iKeySize);
	int		DataSize(void* pvKey, int iKeySize);
	BOOL	Has(void* pvKey, int iKeySize);
	BOOL	Evict(void* pvKey, int iKeySize);
	BOOL	Flush(void* pvKey, int iKeySize);
};


#endif // !__INDEX_TREE_EVICTING_ACCESS_H__


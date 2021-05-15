#ifndef __MAP_INDEX_ACCESS_H__
#define __MAP_INDEX_ACCESS_H__
#include "IndexAccess.h"


class CMapBlock;
class CMapIndexAccess : public CIndexAccess
{
protected:
	CMapBlock*	mpcMap;

public:
	void				Init(CMapBlock* mpcMap);
	BOOL				Kill(void);

	BOOL				Flush(void);
	int64				NumElements(void);
	BOOL				ValidateIndex(void);
	void				Dump(void);

	CIndexTreeIterator* CreateIterator(void);
	void				FreeIterator(CIndexTreeIterator* pcIter) ;

protected:
	BOOL	Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
	BOOL	Get(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize);
	int		DataSize(void* pvKey, int iKeySize);
	BOOL	Remove(void* pvKey, int iKeySize);
	BOOL	Has(void* pvKey, int iKeySize);

	BOOL	Flush(void* pvKey, int iKeySize);
	BOOL	Evict(void* pvKey, int iKeySize);
};


#endif // __MAP_INDEX_ACCESS_H__


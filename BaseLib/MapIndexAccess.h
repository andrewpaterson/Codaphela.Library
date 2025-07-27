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
	bool				Kill(void);

	bool				Flush(void);
	int64				NumElements(void);
	bool				ValidateIndex(void);
	void				Dump(void);

	CIndexTreeIterator* CreateIterator(void);
	void				FreeIterator(CIndexTreeIterator* pcIter) ;

protected:
	bool	Put(uint8* pvKey, size iKeySize, void* pvData, size iDataSize);
	bool	Get(uint8* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize);
	size	DataSize(uint8* pvKey, size iKeySize);
	bool	Remove(uint8* pvKey, size iKeySize);
	bool	Has(uint8* pvKey, size iKeySize);

	bool	Flush(uint8* pvKey, size iKeySize);
	bool	Evict(uint8* pvKey, size iKeySize);
};


#endif // __MAP_INDEX_ACCESS_H__


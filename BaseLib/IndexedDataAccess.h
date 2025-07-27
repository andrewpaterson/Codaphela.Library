#ifndef __INDEXED_DATA_ACCESS_H__
#define __INDEXED_DATA_ACCESS_H__
#include "IndexAccess.h"
#include "IndexedData.h"

class CIndexedDataAccess : public CIndexAccess
{
protected:
	CIndexedData*	mpcIndexData;

public:
	bool				Init(CIndexedData* pcIndexData);
	bool				Kill(void);
	bool				Flush(void);
	int64				NumElements(void);
	bool				ValidateIndex(void);
	void				Dump(void);

	CIndexTreeIterator* CreateIterator(void);
	void				FreeIterator(CIndexTreeIterator* pcIter);

protected:
	bool	Put(uint8* pvKey, size iKeySize, void* pvData, size iDataSize);
	bool	Get(uint8* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize);
	size	DataSize(uint8* pvKey, size iKeySize);
	bool	Remove(uint8* pvKey, size iKeySize);
	bool	Has(uint8* pvKey, size iKeySize);
	bool	Flush(uint8* pvKey, size iKeySize);
	bool	Evict(uint8* pvKey, size iKeySize);
};


#endif // __INDEXED_DATA_ACCESS_H__


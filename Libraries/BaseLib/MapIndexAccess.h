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
	bool				Kill(void) override;

	bool				Flush(void) override;
	int64				NumElements(void) const override;
	bool				ValidateIndex(void) override;
	void				Dump(void) override;

	CIndexTreeIterator*	CreateIterator(void) override;
	void				FreeIterator(CIndexTreeIterator* pcIter) override ;

protected:
	bool	Put(uint8* pvKey, size iKeySize, const void* pvData, size iDataSize) override;
	bool	Get(uint8* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize) override;
	size	DataSize(uint8* pvKey, size iKeySize) override;
	bool	Remove(uint8* pvKey, size iKeySize) override;
	bool	Has(uint8* pvKey, size iKeySize) override;

	bool	Flush(uint8* pvKey, size iKeySize) override;
	bool	Evict(uint8* pvKey, size iKeySize) override;
};


#endif // __MAP_INDEX_ACCESS_H__


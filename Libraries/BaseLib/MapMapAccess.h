#ifndef __MAP_MAP_ACCESS_H__
#define __MAP_MAP_ACCESS_H__
#include "MapAccess.h"


class CMapBlock;
class CMapMapAccess : public CMapAccess
{
protected:
	CMapBlock* mpcMap;

public:
	void			Init(CMapBlock* pcMap);
	void			Kill(void) override;

	int64			NumElements(void) const override;
	void			Dump(void) override;

	CMapIterator*	CreateIterator(void) override;
	void			FreeIterator(CMapIterator* pcIter) override;

protected:
	void*			Put(const void* pvKey, size iKeySize, const void* pvData, size uiDataSize) override;
	void*			Get(const void* pvKey, size iKeySize, size* puiDataSize) override;
	size			DataSize(void* pvKey, size iKeySize) override;
	bool			Remove(const void* pvKey, size iKeySize) override;
	bool			Has(const void* pvKey, size iKeySize) override;
};


#endif // __MAP_INDEX_ACCESS_H__


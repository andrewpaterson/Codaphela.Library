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
	void			Kill(void);

	int64			NumElements(void);
	void			Dump(void);

	CMapIterator*	CreateIterator(void);
	void			FreeIterator(CMapIterator* pcIter);

protected:
	 void*			Put(void* pvKey, uint32 iKeySize, void* pvData, uint32 uiDataSize);
	 void*			Get(void* pvKey, uint32 iKeySize, uint32* puiDataSize);
	 uint32			DataSize(void* pvKey, uint32 iKeySize);
	 bool			Remove(void* pvKey, uint32 iKeySize);
	 bool			Has(void* pvKey, uint32 iKeySize);
};


#endif // __MAP_INDEX_ACCESS_H__


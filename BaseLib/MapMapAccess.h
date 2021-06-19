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
	 void*			Put(void* pvKey, int iKeySize, void* pvData, size_t uiDataSize);
	 void*			Get(void* pvKey, int iKeySize, size_t* puiDataSize);
	 size_t			DataSize(void* pvKey, int iKeySize);
	 BOOL			Remove(void* pvKey, int iKeySize);
	 BOOL			Has(void* pvKey, int iKeySize);
};


#endif // !__MAP_INDEX_ACCESS_H__


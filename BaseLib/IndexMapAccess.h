#ifndef _INDEX_MAP_ACCESS_H__
#define _INDEX_MAP_ACCESS_H__
#include "MapAccess.h"



class CIndexBlock;
class CIndexMapAccess : public CMapAccess
{
protected:
	CIndexBlock*	mpcIndex;

public:
	void			Init(CIndexBlock* pcIndex);
	void			Kill(void);

	int64			NumElements(void);
	void			Dump(void);

	CMapIterator*	CreateIterator(void);
	void			FreeIterator(CMapIterator* pcIter);

protected:
	 void*			Put(void* pvKey, int iKeySize, void* pvData, size_t uiDataSize);
	 void*			Get(void* pvKey, int iKeySize, size_t* puiDataSize);
	 size_t			DataSize(void* pvKey, int iKeySize);
	 bool			Remove(void* pvKey, int iKeySize);
	 bool			Has(void* pvKey, int iKeySize);
};


#endif // _INDEX_MAP_ACCESS_H__


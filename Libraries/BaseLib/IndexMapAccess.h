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


#endif // _INDEX_MAP_ACCESS_H__


#ifndef __MAP_MAP_ITERATOR_H__
#define __MAP_MAP_ITERATOR_H__
#include "MapBlock.h"
#include "MapIterator.h"


class CMapMapIterator : public CMapIterator
{
protected:
	CMapBlock*		mpcMap;
	bool			mbStarted;
	SMapIterator	msIter;

public:
	void	Init(CMapBlock* pcMapBlock);
	void	Kill(void);

	void*	Iterate(uint32* piDataSize = NULL, void* pvDestKey = NULL, uint32* puiKeySize = NULL, uint32 uiMaxKeySize = 0);
};


#endif // __MAP_MAP_ITERATOR_H__


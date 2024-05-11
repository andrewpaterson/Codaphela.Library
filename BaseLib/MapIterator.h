#ifndef __MAP_ITERATOR_H__
#define __MAP_ITERATOR_H__
#include "Define.h"
#include "Killable.h"


class CMapIterator : public CKillable
{
	virtual void*	Iterate(size* piDataSize = NULL, void* pvDestKey = NULL, size* puiKeySize = NULL, size uiMaxKeySize = 0) =0;
};


#endif // __MAP_ITERATOR_H__


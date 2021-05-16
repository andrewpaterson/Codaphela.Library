#ifndef __MAP_ITERATOR_H__
#define __MAP_ITERATOR_H__
#include "Define.h"
#include "Killable.h"


class CMapIterator : public CKillable
{
	virtual void*	Iterate(size_t* piDataSize = NULL, void* pvDestKey = NULL, size_t* puiKeySize = NULL, size_t uiMaxKeySize = 0) =0;
};


#endif // __MAP_ITERATOR_H__


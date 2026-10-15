#ifndef __MAP_STRING_INT_AND_POINTER_H__
#define __MAP_STRING_INT_AND_POINTER_H__
#include "IntAndPointer.h"
#include "MapStringTemplate.h"


typedef CMapStringTemplate<SIntAndPointer> __CMapStringIntAndPointer;


class CMapStringIntAndPointer : public __CMapStringIntAndPointer
{
public:
	SIntAndPointer*		Get(const char* szKey);
	void*				Get(const char* szKey, uint* puiType);

	SIntAndPointer*		Put(const char* szKey, uint uiType, void* pvData);

	bool				StartIteration(SMapIterator* psIterator, void** ppvKey, size* piKeySize, void** ppvData, uint* puiType = NULL);
	bool				Iterate(SMapIterator* psIterator, void** ppvKey, size* piKeySize, void** ppvData, uint* puiType = NULL);
};


#endif // __MAP_STRING_INT_AND_POINTER_H__


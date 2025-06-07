#ifndef __MAP_STRING_INT_AND_POINTER_H__
#define __MAP_STRING_INT_AND_POINTER_H__
#include "IntAndPointer.h"
#include "MapStringTemplate.h"


typedef CMapStringTemplate<SIntAndPointer> __CMapStringIntAndPointer;


class CMapStringIntAndPointer : public __CMapStringIntAndPointer
{
public:
	SIntAndPointer*		Get(char* szKey);
	SIntAndPointer*		Get(const char* szKey);
	void*				Get(char* szKey, uint* puiType);
	void*				Get(const char* szKey, uint* puiType);

	SIntAndPointer*		Put(char* szKey, uint uiType, void* pvData);
	SIntAndPointer*		Put(const char* szKey, uint uiType, void* pvData);
};


#endif // __MAP_STRING_INT_AND_POINTER_H__


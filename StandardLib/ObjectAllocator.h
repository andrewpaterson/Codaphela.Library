#ifndef __OBJECT_ALLOCATOR_H__
#define __OBJECT_ALLOCATOR_H__
#include "Pointer.h"


//This is a private style helper class for allocating objects with a view to either over-writing or re-allocating existing objects that share the index being allocated.
//If you just want to alllocate an object then use Objects.Add
class CObjects;
class CObjectAllocator
{
protected:
	CObjects*	mpcObjects;
	BOOL		mbOverwriteExisting;

public:
	void				Init(CObjects* pcObjects, BOOL bOverwriteExisting);
	void				Kill(void);

	CPointerObject		Add(char* szClassName);
	CPointerObject		Add(char* szClassName, OIndex oiForced);

	CPointerObject		Add(char* szClassName, char* szObjectName);
	CPointerObject		Add(char* szClassName, char* szObjectName, OIndex* poiExisting);
	CPointerObject		Add(char* szClassName, char* szObjectName, OIndex oiForced);
	CPointerObject		Add(char* szClassName, char* szObjectName, OIndex oiForced, OIndex* poiExisting);

	CPointerObject		AddHollow(char* szObjectName);
	CPointerObject		AddHollow(OIndex oiForced);
	CPointerObject		AddHollow(char* szObjectName, OIndex oiForced);
};


#endif // __OBJECT_ALLOCATOR_H__


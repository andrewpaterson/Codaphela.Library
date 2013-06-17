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

public:
	void				Init(CObjects* pcObjects);
	void				Kill(void);

	CPointer		Add(char* szClassName);
	CPointer		Add(char* szClassName, OIndex oiForced);

	CPointer		Add(char* szClassName, char* szObjectName);
	CPointer		Add(char* szClassName, char* szObjectName, OIndex* poiExisting);
	CPointer		Add(char* szClassName, char* szObjectName, OIndex oiForced);
	CPointer		Add(char* szClassName, char* szObjectName, OIndex oiForced, OIndex* poiExisting);

	CPointer		AddHollow(char* szObjectName);
	CPointer		AddHollow(OIndex oiForced);
	CPointer		AddHollow(char* szObjectName, OIndex oiForced);

protected:
	CPointer ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, char* szObjectName, OIndex oiForced);
};


#endif // __OBJECT_ALLOCATOR_H__


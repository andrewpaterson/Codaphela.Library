#ifndef __OBJECT_ALLOCATOR_H__
#define __OBJECT_ALLOCATOR_H__
#include "Pointer.h"


//When are we:
//  Allocating an object that already exists on disk...
//    With a Name and Index?
//    With an Index?
//  Allocating an object that does not exist at all?
//  Allocating and overwriting an object that already exists on disk...
//    With a Name?
//  Allocating and overwriting an object that already exists in memory...
//    With a Name?


//This is a private style helper class for allocating objects with a view to either over-writing or re-allocating existing objects that share the index being allocated.
//If you just want to alllocate an object then use Objects.Add
class CObjects;
class CObjectAllocator
{
protected:
	CObjects*	mpcObjects;

public:
	void			Init(CObjects* pcObjects);
	void			Kill(void);

	CBaseObject*	AllocateNew(char* szClassName);
	CBaseObject*	AllocateNewMaybeReplaceExisting(char* szClassName, char* szObjectName);

	CBaseObject*	AllocateNewMaybeReplaceExisting(char* szClassName, OIndex oiForced);
	CBaseObject*	AllocateNewMaybeReplaceExisting(char* szClassName, char* szObjectName, OIndex oiForced);

	CBaseObject*	AllocateExistingHollowFromMemoryOrMaybeANewNamedHollow(char* szObjectName, uint16 iNumEmbedded);
	CBaseObject*	AllocateExistingHollow(OIndex oiForced, uint16 iNumEmbedded);
	CBaseObject*	AllocateExistingHollow(char* szObjectName, OIndex oiForced, uint16 iNumEmbedded);

protected:
	CBaseObject*	ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, char* szObjectName, OIndex oiForced);
	CBaseObject*	ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, OIndex oiForced);
};


#endif // __OBJECT_ALLOCATOR_H__


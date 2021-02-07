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

	CBaseObject*	Add(char* szClassName);
	CBaseObject*	Add(char* szClassName, OIndex oiForced);

	CBaseObject*	Add(char* szClassName, char* szObjectName);
	CBaseObject*	Add(char* szClassName, char* szObjectName, OIndex* poiExisting);
	CBaseObject*	Add(char* szClassName, char* szObjectName, OIndex oiForced);
	CBaseObject*	Add(char* szClassName, char* szObjectName, OIndex oiForced, OIndex* poiExisting);

	CBaseObject*	AddHollow(char* szObjectName, uint16 iNumEmbedded);
	CBaseObject*	AddHollow(OIndex oiForced, uint16 iNumEmbedded);
	CBaseObject*	AddHollow(char* szObjectName, OIndex oiForced, uint16 iNumEmbedded);

protected:
	CBaseObject*	ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, char* szObjectName, OIndex oiForced);
	CBaseObject*	ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, OIndex oiForced);
};


#endif // __OBJECT_ALLOCATOR_H__


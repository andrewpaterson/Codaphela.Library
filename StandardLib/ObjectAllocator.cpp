#include "BaseLib/Log.h"
#include "ObjectAllocator.h"
#include "HollowObject.h"
#include "NamedHollowObject.h"
#include "ObjectRemapFrom.h"
#include "Objects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectAllocator::Init(CObjects* pcObjects)
{
	mpcObjects = pcObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectAllocator::Kill(void)
{
	mpcObjects = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AllocateNew(char* szClassName)
{
	return mpcObjects->AllocateNew(szClassName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AllocateNewMaybeReplaceExisting(char* szClassName, OIndex oiForced)
{
	return mpcObjects->AllocateNewMaybeReplaceExisting(szClassName, oiForced);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AllocateNewMaybeReplaceExisting(char* szClassName, char* szObjectName)
{
	return mpcObjects->AllocateNewMaybeReplaceExisting(szClassName, szObjectName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AllocateNewMaybeReplaceExisting(char* szClassName, char* szObjectName, OIndex oiForced)
{
	return mpcObjects->AllocateNewMaybeReplaceExisting(szClassName, szObjectName, oiForced);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, char* szObjectName, OIndex oiForced)
{
	return mpcObjects->ReplaceExisting(pvExisting, pvObject, szObjectName, oiForced);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, OIndex oiForced)
{
	return mpcObjects->ReplaceExisting(pvExisting, pvObject, oiForced);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AllocateExistingHollow(OIndex oiForced, uint16 iNumEmbedded)
{
	return mpcObjects->AllocateExistingHollow(oiForced, iNumEmbedded);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AllocateExistingHollow(char* szObjectName, OIndex oiForced, uint16 iNumEmbedded)
{
	return mpcObjects->AddHollow(szObjectName, oiForced, iNumEmbedded);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AllocateExistingHollowFromMemoryOrMaybeANewNamedHollow(char* szObjectName, uint16 iNumEmbedded)
{
	return mpcObjects->AllocateExistingHollowFromMemoryOrMaybeANewNamedHollow(szObjectName, iNumEmbedded);
}


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
CBaseObject* CObjectAllocator::Add(char* szClassName)
{
	return mpcObjects->Add(szClassName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::Add(char* szClassName, OIndex oiForced)
{
	return mpcObjects->Add(szClassName, oiForced);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::Add(char* szClassName, char* szObjectName)
{
	return mpcObjects->Add(szClassName, szObjectName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex* poiExisting)
{
	return mpcObjects->Add(szClassName, szObjectName, poiExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oiForced)
{
	return mpcObjects->Add(szClassName, szObjectName, oiForced);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oiForced, OIndex* poiExisting)
{
	return mpcObjects->Add(szClassName, szObjectName, oiForced, poiExisting);
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
CBaseObject* CObjectAllocator::AddHollow(OIndex oiForced, uint16 iNumEmbedded)
{
	return mpcObjects->AddHollow(oiForced, iNumEmbedded);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AddHollow(char* szObjectName, OIndex oiForced, uint16 iNumEmbedded)
{
	return mpcObjects->AddHollow(szObjectName, oiForced, iNumEmbedded);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AddHollow(char* szObjectName, uint16 iNumEmbedded)
{
	return mpcObjects->AddHollow(szObjectName, iNumEmbedded);
}


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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::Add(char* szClassName)
{
	CBaseObject*	pvObject;

	pvObject = mpcObjects->Allocate(szClassName);
	if (!pvObject)
	{
		return NULL;
	}
	
	mpcObjects->AddWithIDAndName(pvObject, NULL, mpcObjects->GetIndexGenerator()->PopIndex());
	LOG_OBJECT_ALLOCATION(pvObject);

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::Add(char* szClassName, OIndex oiForced)
{
	CBaseObject*	pvExisting;
	CBaseObject*	pvObject;
	BOOL			bResult;

	pvObject = mpcObjects->Allocate(szClassName);
	if (!pvObject)
	{
		return NULL;
	}

	pvExisting = mpcObjects->GetFromMemory(oiForced);
	if (pvExisting == NULL)
	{
		bResult = mpcObjects->AddWithIDAndName(pvObject, NULL, oiForced);
		if (!bResult)
		{
			pvObject->Kill();
			return NULL;
		}
		LOG_OBJECT_ALLOCATION(pvObject);

		return pvObject;
	}
	else
	{
		pvObject = ReplaceExisting(pvExisting, pvObject, NULL, oiForced);
		LOG_OBJECT_ALLOCATION(pvObject);
		
		return pvObject;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::Add(char* szClassName, char* szObjectName)
{
	OIndex oiExisting;

	return Add(szClassName, szObjectName, &oiExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex* poiExisting)
{
	CBaseObject*	pvObject;
	OIndex			oi;

	oi = mpcObjects->GetIndexGenerator()->PopIndex();
	pvObject = Add(szClassName, szObjectName, oi, poiExisting);

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oiForced)
{
	OIndex oiExisting;

	return Add(szClassName, szObjectName, oiForced, &oiExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oiForced, OIndex* poiExisting)
{
	CBaseObject*	pvObject;
	CBaseObject*	pvExisting;
	BOOL			bResult;

	pvObject = mpcObjects->Allocate(szClassName);
	if (!pvObject)
	{
		gcLogger.Error2(__METHOD__, " Cannot add object named [", szObjectName, "] class [", szClassName, "].", NULL);
		return NULL;
	}

	if (!pvObject->IsNamed())
	{
		gcLogger.Error2(__METHOD__, " Cannot add object named [", szObjectName, "] the class ", pvObject->ClassName(), " is not derived from NamedObject.", NULL);
		pvObject->Kill();
		return NULL;
	}

	pvExisting = mpcObjects->GetFromMemory(szObjectName);
	if (pvExisting == NULL)
	{
		bResult = mpcObjects->AddWithIDAndName(pvObject, szObjectName, oiForced);
		if (!bResult)
		{
			pvObject->Kill();
			return NULL;
		}
		LOG_OBJECT_ALLOCATION(pvObject);

		*poiExisting = INVALID_O_INDEX;
		return pvObject;
	}
	else
	{
		*poiExisting = pvExisting->GetOI();
		pvObject = ReplaceExisting(pvExisting, pvObject, szObjectName, oiForced);
		LOG_OBJECT_ALLOCATION(pvObject);

		return pvObject;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, char* szObjectName, OIndex oiForced)
{
	BOOL				bResult;
	CObjectRemapFrom	cRemapper;

	mpcObjects->Dename(pvExisting);
	mpcObjects->Deindex(pvExisting);

	bResult = mpcObjects->AddWithIDAndName(pvObject, szObjectName, oiForced);
	if (!bResult)
	{
		pvObject->Kill();
		return NULL;
	}

	cRemapper.Remap(pvExisting, pvObject);
	pvExisting->ClearIndex();
	pvExisting->Kill();

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AddHollow(OIndex oiForced, unsigned short iNumEmbedded)
{
	CHollowObject*		pcHollow;
	BOOL				bResult;
	CBaseObject*		pcExisting;

	if (oiForced == INVALID_O_INDEX)
	{
		gcLogger.Error("CObjectAllocator::AddHollow Cannot allocate a hollow object with an invalid index.");
		return NULL;
	}

	pcExisting = mpcObjects->GetFromMemory(oiForced);
	if (pcExisting)
	{
		LOG_OBJECT_ALLOCATION(pcExisting);
		return pcExisting;
	}

	pcHollow = mpcObjects->AllocateHollow(iNumEmbedded);
	if (!pcHollow)
	{
		return NULL;
	}

	bResult = mpcObjects->AddWithIDAndName(pcHollow, NULL, oiForced);
	if (bResult)
	{
		LOG_OBJECT_ALLOCATION(pcHollow);
		return pcHollow;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AddHollow(char* szObjectName, OIndex oiForced, unsigned short iNumEmbedded)
{
	CNamedHollowObject*			pcHollow;
	BOOL						bResult;
	CBaseObject*				pvExisting;

	if (oiForced == INVALID_O_INDEX)
	{
		gcLogger.Error("CObjectAllocator::AddHollow Cannot allocate a hollow object with an invalid index.");
		return NULL;
	}

	if ((szObjectName == NULL || szObjectName[0] == '\0'))
	{
		return AddHollow(oiForced, iNumEmbedded);
	}

	pvExisting = mpcObjects->GetFromMemory(szObjectName);
	if (pvExisting)
	{
		return pvExisting;
	}

	pvExisting = mpcObjects->GetFromMemory(oiForced);
	if (pvExisting)
	{
		gcLogger.Error2(__METHOD__, " Cannot add hollow object named [", szObjectName, "] another object with index [", IndexToString(oiForced), "] and name [", pvExisting->GetName(), "] already exists.", NULL);
		return NULL;
	}

	pcHollow = mpcObjects->AllocateNamedHollow(iNumEmbedded);
	if (!pcHollow)
	{
		return NULL;
	}

	bResult = gcObjects.AddWithIDAndName(pcHollow, szObjectName, oiForced);
	if (bResult)
	{
		LOG_OBJECT_ALLOCATION(pcHollow);
		return pcHollow;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::AddHollow(char* szObjectName, unsigned short iNumEmbedded)
{
	CNamedHollowObject*		pcHollow;
	BOOL					bResult;
	CBaseObject*			pvExisting;

	if ((szObjectName == NULL || szObjectName[0] == '\0'))
	{
		return NULL;
	}

	pvExisting = mpcObjects->GetFromMemory(szObjectName);
	if (pvExisting)
	{
		return pvExisting;
	}

	pcHollow = mpcObjects->AllocateNamedHollow(iNumEmbedded);
	if (!pcHollow)
	{
		return NULL;
	}

	pcHollow->InitName(szObjectName);

	bResult = gcObjects.AddWithIDAndName(pcHollow, szObjectName, mpcObjects->GetIndexGenerator()->PopIndex());
	if (bResult)
	{
		LOG_OBJECT_ALLOCATION(pcHollow);
		return pcHollow;
	}
	else
	{
		return NULL;
	}
}

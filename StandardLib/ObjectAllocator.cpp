#include "BaseLib/Logger.h"
#include "ObjectAllocator.h"
#include "HollowObject.h"
#include "NamedHollowObject.h"
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

		return pvObject;
	}
	else
	{
		return ReplaceExisting(pvExisting, pvObject, NULL, oiForced);
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
		gcLogger.Error2("CObjectAllocator::Add cannot add object named [", szObjectName, "] class [", szClassName, "].", NULL);
		return NULL;
	}

	if (!pvObject->IsNamed())
	{
		gcLogger.Error2("CObjectAllocator::Add cannot add object named [", szObjectName, "] the class ", pvObject->ClassName(), " is not derived from NamedObject.", NULL);
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

		*poiExisting = INVALID_O_INDEX;

		return pvObject;
	}
	else
	{
		*poiExisting = pvExisting->GetOI();
		return ReplaceExisting(pvExisting, pvObject, szObjectName, oiForced);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectAllocator::ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, char* szObjectName, OIndex oiForced)
{
	BOOL bResult;

	mpcObjects->Dename(pvExisting);
	mpcObjects->Deindex(pvExisting);
	bResult = mpcObjects->AddWithIDAndName(pvObject, szObjectName, oiForced);
	if (!bResult)
	{
		pvObject->Kill();
		return NULL;
	}

	CPointer	pObject; //Fix me

	pObject.AssignObject(pvObject);

	pObject.RemapFrom(pvExisting);  //Remap from does not belong on Pointer.

	pvExisting->ClearIndex();
	pvExisting->Kill();

	return (CBaseObject*)pObject.Object();  //Fix me
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
		gcLogger.Error2("CObjectAllocator::AddHollow cannot add hollow object named [", szObjectName, "] another object with index [", IndexToString(oiForced), "] and name [", pvExisting->GetName(), "] already exists.", NULL);
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
		return pcHollow;
	}
	else
	{
		return NULL;
	}
}

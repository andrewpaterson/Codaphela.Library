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
CPointer CObjectAllocator::Add(char* szClassName)
{
	CBaseObject*	pvObject;

	pvObject = mpcObjects->Allocate(szClassName);
	if (!pvObject)
	{
		return ONull;
	}

	CPointer	pObject;

	mpcObjects->AddWithIDAndName(pvObject, NULL, mpcObjects->GetIndexGenerator()->PopIndex());

	//No PointTo because we don't know the embedding object until assignment.
	pObject.mpcObject = pvObject;
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjectAllocator::Add(char* szClassName, OIndex oiForced)
{
	CBaseObject*	pvExisting;
	CBaseObject*	pvObject;
	BOOL			bResult;

	pvObject = mpcObjects->Allocate(szClassName);
	if (!pvObject)
	{
		return ONull;
	}

	CPointer	pObject;

	CPointer	pExistingObject;

	pExistingObject = mpcObjects->GetFromMemory(oiForced);
	if (pExistingObject.IsNull())
	{
		bResult = mpcObjects->AddWithIDAndName(pvObject, NULL, oiForced);
		if (!bResult)
		{
			pvObject->Kill();
			return ONull;
		}

		pObject.mpcObject = pvObject;
		return pObject;
	}
	else
	{
		pvExisting = pExistingObject.Object();
		return ReplaceExisting(pvExisting, pvObject, NULL, oiForced);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjectAllocator::Add(char* szClassName, char* szObjectName)
{
	OIndex oiExisting;

	return Add(szClassName, szObjectName, &oiExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex* poiExisting)
{
	CPointer	pObject;
	OIndex			oi;

	oi = mpcObjects->GetIndexGenerator()->PopIndex();
	pObject = Add(szClassName, szObjectName, oi, poiExisting);

	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oiForced)
{
	OIndex oiExisting;

	return Add(szClassName, szObjectName, oiForced, &oiExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oiForced, OIndex* poiExisting)
{
	CBaseObject*	pvObject;
	CBaseObject*	pvExisting;
	BOOL			bResult;

	pvObject = mpcObjects->Allocate(szClassName);
	if (!pvObject)
	{
		return ONull;
	}

	if (!pvObject->IsNamed())
	{
		gcLogger.Error2("CObjectAllocator::AddHollow cannot object named [", szObjectName, "] the class ", pvObject->ClassName(), " is not derived from NamedObject.", NULL);
		pvObject->Kill();
		return ONull;
	}

	CPointer	pExistingObject;

	pExistingObject = mpcObjects->GetFromMemory(szObjectName);
	if (pExistingObject.IsNull())
	{
		bResult = mpcObjects->AddWithIDAndName(pvObject, szObjectName, oiForced);
		if (!bResult)
		{
			pvObject->Kill();
			return ONull;
		}

		CPointer	pObject;

		*poiExisting = INVALID_O_INDEX;
		pObject.mpcObject = pvObject;
		return pObject;
	}
	else
	{
		*poiExisting = pExistingObject.GetIndex();
		pvExisting = pExistingObject.Object();
		return ReplaceExisting(pvExisting, pvObject, szObjectName, oiForced);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjectAllocator::ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, char* szObjectName, OIndex oiForced)
{
	BOOL bResult;

	mpcObjects->Dename(pvExisting);
	mpcObjects->Deindex(pvExisting);
	bResult = mpcObjects->AddWithIDAndName(pvObject, szObjectName, oiForced);
	if (!bResult)
	{
		pvObject->Kill();
		return ONull;
	}

	CPointer	pObject;

	pObject.mpcObject = pvObject;

	pObject.RemapFrom(pvExisting);

	pvExisting->ClearIndex();
	pvExisting->Kill();

	return pObject;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjectAllocator::AddHollow(OIndex oiForced)
{
	Ptr<CHollowObject>		pHollow;
	CHollowObject*				pcHollow;
	BOOL						bResult;
	CPointer				pcExisting;

	pcExisting = mpcObjects->GetFromMemory(oiForced);
	if (pcExisting.IsNotNull())
	{
		return pcExisting;
	}

	pcHollow = mpcObjects->Allocate<CHollowObject>();
	if (!pcHollow)
	{
		return ONull;
	}

	bResult = mpcObjects->AddWithIDAndName(pcHollow, NULL, oiForced);
	if (bResult)
	{
		pHollow.mpcObject = pcHollow;
		return pHollow;
	}
	else
	{
		return ONull;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjectAllocator::AddHollow(char* szObjectName, OIndex oiForced)
{
	Ptr<CNamedHollowObject>	pHollow;
	CNamedHollowObject*				pcHollow;
	BOOL							bResult;
	CPointer					pcExisting;

	if ((szObjectName == NULL || szObjectName[0] == '\0'))
	{
		return AddHollow(oiForced);
	}

	pcExisting = mpcObjects->GetFromMemory(szObjectName);
	if (pcExisting.IsNotNull())
	{
		return pcExisting;
	}

	pcExisting = mpcObjects->GetFromMemory(oiForced);
	if (pcExisting.IsNotNull())
	{
		gcLogger.Error2("CObjectAllocator::AddHollow cannot add hollow object named [", szObjectName, "] another object with index [", IndexToString(oiForced), "] and name [", pcExisting.GetName(), "] already exists.", NULL);
		return ONull;
	}

	pcHollow = mpcObjects->Allocate<CNamedHollowObject>();
	if (!pcHollow)
	{
		return ONull;
	}

	pcHollow->InitName(szObjectName);

	bResult = gcObjects.AddWithIDAndName(pcHollow, szObjectName, oiForced);
	if (bResult)
	{
		pHollow.mpcObject = pcHollow;
		return pHollow;
	}
	else
	{
		return ONull;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjectAllocator::AddHollow(char* szObjectName)
{
	Ptr<CNamedHollowObject>	pHollow;
	CNamedHollowObject*				pcHollow;
	BOOL							bResult;
	CPointer					pcExisting;

	if ((szObjectName == NULL || szObjectName[0] == '\0'))
	{
		return ONull;
	}

	pcExisting = mpcObjects->GetFromMemory(szObjectName);
	if (pcExisting.IsNotNull())
	{
		return pcExisting;
	}

	pcHollow = mpcObjects->Allocate<CNamedHollowObject>();
	if (!pcHollow)
	{
		return ONull;
	}

	pcHollow->InitName(szObjectName);

	bResult = gcObjects.AddWithIDAndName(pcHollow, szObjectName, mpcObjects->GetIndexGenerator()->PopIndex());
	if (bResult)
	{
		pHollow.mpcObject = pcHollow;
		return pHollow;
	}
	else
	{
		return ONull;
	}
}

#include "BaseLib/Logger.h"
#include "ObjectAllocator.h"
#include "HollowObject.h"
#include "NamedHollowObject.h"
#include "Objects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectAllocator::Init(CObjects* pcObjects, BOOL bOverwriteExisting)
{
	mpcObjects = pcObjects;
	mbOverwriteExisting = bOverwriteExisting;
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
CPointerObject CObjectAllocator::Add(char* szClassName)
{
	CBaseObject*	pvObject;

	pvObject = mpcObjects->Allocate(szClassName);
	if (pvObject)
	{
		CPointerObject	pObject;

		if (!pvObject->IsNamed())
		{
			mpcObjects->AddWithID(pvObject, mpcObjects->GetIndexGenerator()->PopIndex());
		}
		else
		{
			mpcObjects->AddWithIDAndName(pvObject, NULL, mpcObjects->GetIndexGenerator()->PopIndex());
		}

		//No PointTo because we don't know the embedding object until assignment.
		pObject.mpcObject = pvObject;
		return pObject;
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
CPointerObject CObjectAllocator::Add(char* szClassName, OIndex oiForced)
{
	CBaseObject*	pvObject;
	BOOL			bResult;

	pvObject = mpcObjects->Allocate(szClassName);
	if (pvObject)
	{
		CPointerObject	pObject;

		if (mbOverwriteExisting)
		{
			//Check if exists and Dename etc...
		}

		if (!pvObject->IsNamed())
		{
			bResult = mpcObjects->AddWithID(pvObject, oiForced);
		}
		else
		{
			bResult = mpcObjects->AddWithIDAndName(pvObject, NULL, oiForced);
		}

		if (bResult)
		{
			//No PointTo because we don't know the embedding object until assignment.
			pObject.mpcObject = pvObject;
			return pObject;
		}
		else
		{
			pvObject->Kill();
			return ONull;
		}
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
CPointerObject CObjectAllocator::Add(char* szClassName, char* szObjectName)
{
	OIndex oiExisting;

	return Add(szClassName, szObjectName, &oiExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex* poiExisting)
{
	CBaseObject*	pvObject;
	BOOL			bResult;

	pvObject = mpcObjects->Allocate(szClassName);
	if (pvObject)
	{
		if (pvObject->IsNamed())
		{
			CPointerObject	pObject;
			CPointerObject	pExistingObject;

			if (mbOverwriteExisting)
			{
				//Check if exists and Dename etc...
			}

			pExistingObject = mpcObjects->GetIfInMemory(szObjectName);
			if (pExistingObject.IsNull())
			{
				bResult = mpcObjects->AddWithIDAndName(pvObject, szObjectName, mpcObjects->GetIndexGenerator()->PopIndex());
				if (!bResult)
				{
					pvObject->Kill();
					return ONull;
				}
				*poiExisting = INVALID_O_INDEX;
				pObject.mpcObject = pvObject;
				return pObject;
			}
			else
			{
				mpcObjects->Dename(pExistingObject.Object());
				mpcObjects->Deindex(pExistingObject.Object());
				bResult = mpcObjects->AddWithIDAndName(pvObject, szObjectName, mpcObjects->GetIndexGenerator()->PopIndex());
				if (!bResult)
				{
					pvObject->Kill();
					return ONull;
				}

				*poiExisting = pExistingObject.GetIndex();
				pObject.mpcObject = pvObject;

				pObject.RemapFrom(pExistingObject.Object());

				pExistingObject.ClearIndex();
				pExistingObject.Kill();

				return pObject;
			}
		}
		else
		{
			//gcLogger.Error()
			pvObject->Kill();
			return ONull;
		}
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
CPointerObject CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oiForced)
{
	OIndex oiExisting;

	return Add(szClassName, szObjectName, oiForced, &oiExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oiForced, OIndex* poiExisting)
{
	CBaseObject*	pvObject;
	BOOL			bResult;

	pvObject = mpcObjects->Allocate(szClassName);
	if (pvObject)
	{
		if (pvObject->IsNamed())
		{
			CPointerObject	pObject;
			CPointerObject	pExistingObject;

			pExistingObject = mpcObjects->GetIfInMemory(szObjectName);
			if (pExistingObject.IsNull())
			{
				bResult = mpcObjects->AddWithIDAndName(pvObject, szObjectName, oiForced);
				if (!bResult)
				{
					pvObject->Kill();
					return ONull;
				}
				*poiExisting = INVALID_O_INDEX;
				pObject.mpcObject = pvObject;
				return pObject;
			}
			else
			{
				mpcObjects->Dename(pExistingObject.Object());
				mpcObjects->Deindex(pExistingObject.Object());
				bResult = mpcObjects->AddWithIDAndName(pvObject, szObjectName, oiForced);
				if (!bResult)
				{
					pvObject->Kill();
					return ONull;
				}

				*poiExisting = pExistingObject.GetIndex();
				pObject.mpcObject = pvObject;

				pObject.RemapFrom(pExistingObject.Object());

				pExistingObject.ClearIndex();
				pExistingObject.Kill();

				return pObject;
			}
		}
		else
		{
			//gcLogger.Error()
			pvObject->Kill();
			return ONull;
		}
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
CPointerObject CObjectAllocator::AddHollow(OIndex oiForced)
{
	CPointer<CHollowObject>		pHollow;
	CHollowObject*				pcHollow;
	BOOL						bResult;
	CPointerObject				pcExisting;

	pcExisting = mpcObjects->GetIfInMemory(oiForced);
	if (pcExisting.IsNotNull())
	{
		return pcExisting;
	}

	pcHollow = mpcObjects->Allocate<CHollowObject>();
	bResult = mpcObjects->AddWithID(pcHollow, oiForced);
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
CPointerObject CObjectAllocator::AddHollow(char* szObjectName, OIndex oiForced)
{
	CPointer<CNamedHollowObject>	pHollow;
	CNamedHollowObject*				pcHollow;
	BOOL							bResult;
	CPointerObject					pcExisting;

	pcExisting = mpcObjects->GetIfInMemory(szObjectName);
	if (pcExisting.IsNotNull())
	{
		return pcExisting;
	}

	pcExisting = mpcObjects->GetIfInMemory(oiForced);
	if (pcExisting.IsNotNull())
	{
		gcLogger.Error2("CObjectAllocator::AddHollow cannot add hollow object named [", szObjectName, "] another object with index [", IndexToString(oiForced), "] already exists.", NULL);
		return ONull;
	}

	pcHollow = mpcObjects->Allocate<CNamedHollowObject>();
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
CPointerObject CObjectAllocator::AddHollow(char* szObjectName)
{
	CPointer<CNamedHollowObject>	pHollow;
	CNamedHollowObject*				pcHollow;
	BOOL							bResult;
	CPointerObject					pcExisting;

	pcExisting = mpcObjects->GetIfInMemory(szObjectName);
	if (pcExisting.IsNotNull())
	{
		return pcExisting;
	}

	pcHollow = mpcObjects->Allocate<CNamedHollowObject>();
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

#include "ObjectAllocator.h"
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
	CPointerObject	pObject;
	CBaseObject*	pvObject;

	pvObject = mpcObjects->Allocate(szClassName);
	if (!pvObject)
	{
		return ONull;
	}

	if (!pvObject->IsNamed())
	{
		mpcObjects->AddWithID(pvObject);
	}
	else
	{
		mpcObjects->AddWithIDAndName(pvObject, NULL);
	}

	//No PointTo because we don't know the embedding object until assignment.
	pObject.mpcObject = pvObject;
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectAllocator::Add(char* szClassName, char* szObjectName)
{
	CPointerObject	pObject;
	CBaseObject*	pvObject;

	pvObject = mpcObjects->Allocate(szClassName);
	if (!pvObject->IsNamed())
	{
		mpcObjects->	AddWithIDAndName(pvObject, szObjectName);
	}
	else
	{
		return Null();
	}

	//No PointTo because we don't know the embedding object until assignment.
	pObject.mpcObject = pvObject;
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectAllocator::Add(char* szClassName, OIndex oi)
{
	OIndex	oiExisting;

	return Add(szClassName, oi, &oiExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oi)
{
	OIndex	oiExisting;

	return Add(szClassName, szObjectName, oi, &oiExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectAllocator::Add(char* szClassName, OIndex oi, OIndex* poiExisting)
{
	//This method must be removed from objects.

	CBaseObject*	pvObject;
	CBaseObject*	pvExisting;
	BOOL			bResult;

	pvObject = mpcObjects->Allocate(szClassName);
	if (pvObject)
	{
		if (!pvObject->IsNamed())
		{
			CPointerObject	pObject;

			pvExisting = NULL;
			bResult = mpcObjects->AddWithID(pvObject, oi);

			if (!bResult)
			{
				return ONull;
			}


			if (pvExisting)
			{
				bResult = mpcObjects->AddWithID(pvExisting, mpcObjects->GetIndexGenerator()->PopIndex());
				if (!bResult)
				{
					pvObject->Kill();
					return ONull;
				}
				*poiExisting = pvExisting->GetOI();
			}
			else
			{
				*poiExisting = INVALID_O_INDEX;
			}

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
CPointerObject CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oi, OIndex* poiExisting)
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

			pExistingObject = mpcObjects->Get(szObjectName);
			if (pExistingObject.IsNull())
			{
				bResult = mpcObjects->AddWithIDAndName(pvObject, szObjectName, oi);
				if (!bResult)
				{
					return ONull;
				}
				*poiExisting = INVALID_O_INDEX;
			}
			else
			{
				bResult = mpcObjects->AddWithIDAndName(pExistingObject.Object(), szObjectName, mpcObjects->GetIndexGenerator()->PopIndex());
				if (!bResult)
				{
					pvObject->Kill();
					return ONull;
				}
				*poiExisting = pExistingObject.GetIndex();
			}

			pObject.mpcObject = pvObject;
			return pObject;
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


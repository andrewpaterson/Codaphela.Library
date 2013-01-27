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
	if (!pvObject->IsNamed())
	{
		mpcObjects->	AddWithID(pvObject);
	}
	else
	{
		mpcObjects->	AddWithIDAndName(pvObject, NULL);
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

	return mpcObjects->Add(szClassName, oi, &oiExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oi)
{
	OIndex	oiExisting;

	return mpcObjects->Add(szClassName, szObjectName, oi, &oiExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectAllocator::Add(char* szClassName, OIndex oi, OIndex* poiExisting)
{
	return mpcObjects->Add(szClassName, oi, poiExisting);
}


CPointerObject CObjectAllocator::Add(char* szClassName, char* szObjectName, OIndex oi, OIndex* poiExisting)
{
	return mpcObjects->Add(szClassName, szObjectName, oi, poiExisting);
}


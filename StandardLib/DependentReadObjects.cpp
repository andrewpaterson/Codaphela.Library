#include "DependentReadObjects.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::Init(void)
{
	mcObjects.Init(1024);
	miGetIndex = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::Kill(void)
{
	mcObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::Add(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr)
{
	CDependentReadObject	cObject;
	BOOL					bExists;
	int						iIndex;

	cObject.Init(pcHeader, ppcObjectPtr);

	bExists = mcObjects.FindInSorted(&cObject, &CompareDependentReadObject, &iIndex);
	if (!bExists)
	{
		mcObjects.InsertAt(&cObject, iIndex);
	}
	else
	{
		cObject.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDependentReadObject* CDependentReadObjects::GetUnread(void)
{
	int						iOldIndex;
	CDependentReadObject*	psObject;

	if (mcObjects.NumElements() == 0)
	{
		return NULL;
	}

	iOldIndex = miGetIndex;
	for (;;)
	{
		if (miGetIndex >= mcObjects.NumElements()-1)
		{
			miGetIndex = 0;
		}
		else
		{
			miGetIndex++;
		}

		psObject = mcObjects.Get(miGetIndex);
		if (!psObject->mbRead)
		{
			return psObject;
		}

		if (miGetIndex == iOldIndex)
		{
			return NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::SetInitialIndex(OIndex oi)
{
	CDependentReadObject*	pDependent;

	if (mcObjects.NumElements() == 1)
	{
		pDependent = mcObjects.Get(0);
		if (pDependent->moi == INVALID_O_INDEX)
		{
			pDependent->moi = oi;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::Mark(OIndex oi)
{
	CDependentReadObject	cObject;
	int						iIndex;
	BOOL					bResult;
	CDependentReadObject*	pcDependent;

	cObject.moi = oi;

	bResult = mcObjects.FindInSorted(&cObject, &CompareDependentReadObject, &iIndex);
	pcDependent = mcObjects.Get(iIndex);
	pcDependent->mbRead = TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//void CDependentReadObjects::Mark(CBaseObject* pcObject)
//{
//	CDependentReadObject	cObject;
//	int						iIndex;
//
//	cObject.Init(pcObject, TRUE);
//
//	iIndex = mcObjects.InsertIntoSorted(&CompareDependentReadObject, &cObject, TRUE);
//}
//

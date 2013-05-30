#include "Pointer.h"
#include "Objects.h"
#include "IndexGenerator.h"
#include "DependentReadObjects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::Init(void)
{
	mcObjects.Init(128);
	mcPointers.Init(512);
	miGetIndex = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::Kill(void)
{
	int						i;
	CDependentReadObject*	pcDependent;

	mcPointers.Kill();

	for(i = 0; i < mcObjects.NumElements(); i++)
	{
		pcDependent = mcObjects.Get(i);
		pcDependent->Kill();
	}
	mcObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::Add(CPointerHeader* pcHeader, CBaseObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated)
{
	CDependentReadObject	cDependent;
	CDependentReadObject*	pcExistingInFile;
	BOOL					bOiExistsInDependents;
	int						iIndex;
	CDependentReadPointer*	pcPointer;
	CPointerObject			pExisitingInDatabase;
	BOOL					bNameExistsInDatabase;

	cDependent.Init(pcHeader);

	bOiExistsInDependents = mcObjects.FindInSorted(&cDependent, &CompareDependentReadObject, &iIndex);
	if (!bOiExistsInDependents)
	{
		if (pcHeader->IsNamed())
		{
			bNameExistsInDatabase = gcObjects.Contains(pcHeader->mszObjectName.Text());
			if (bNameExistsInDatabase)
			{
				cDependent.SetExisting();
			}
		}

		mcObjects.InsertAt(&cDependent, iIndex);
	}
	else
	{
		pcExistingInFile = mcObjects.Get(iIndex);
		cDependent.Kill();
	}

	pcPointer = mcPointers.Add();
	pcPointer->Init(ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated, pcHeader->moi);
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
		if (!psObject->IsRead())
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
	//This method is *way* dodgy, it exists because the initial addDepenent call made by the ObjectGraphDeserialiser hasn't yet read the serialised object and doesn't know it's Index.

	CDependentReadObject*	pDependent;
	CDependentReadPointer*	pcPointer;

	if ((mcObjects.NumElements() == 1) && (mcPointers.NumElements() == 1))
	{
		pDependent = mcObjects.Get(0);
		if (pDependent->moi == INVALID_O_INDEX)
		{
			pDependent->moi = oi;
		}

		pcPointer = mcPointers.Get(0);
		if (pcPointer->moiPointedTo == INVALID_O_INDEX)
		{
			pcPointer->moiPointedTo = oi;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDependentReadObjects::Mark(OIndex oi)
{
	CDependentReadObject	cObject;
	int						iIndex;
	BOOL					bResult;
	CDependentReadObject*	pcDependent;

	cObject.moi = oi;

	bResult = mcObjects.FindInSorted(&cObject, &CompareDependentReadObject, &iIndex);
	if (!bResult)
	{
		return FALSE;
	}
	pcDependent = mcObjects.Get(iIndex);
	pcDependent->SetRead();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDependentReadObject* CDependentReadObjects::GetObject(OIndex oi)
{
	CDependentReadObject	cObject;
	int						iIndex;
	BOOL					bResult;
	CDependentReadObject*	pcDependent;

	cObject.moi = oi;

	bResult = mcObjects.FindInSorted(&cObject, &CompareDependentReadObject, &iIndex);
	if (!bResult)
	{
		return NULL;
	}
	pcDependent = mcObjects.Get(iIndex);
	return pcDependent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDependentReadObjects::NumPointers(void)
{
	return mcPointers.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDependentReadPointer* CDependentReadObjects::GetPointer(int iIndex)
{
	return mcPointers.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDependentReadObjects::NumObjects(void)
{
	return mcObjects.NumElements();
}


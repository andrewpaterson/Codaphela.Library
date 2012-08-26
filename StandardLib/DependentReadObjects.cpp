#include "DependentReadObjects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadPointer::Init(CBaseObject**	ppcPointedFrom, OIndex oiPointedTo)
{
	mppcPointedFrom = ppcPointedFrom;
	moiPointedTo = oiPointedTo;
}


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
void CDependentReadObjects::Add(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr)
{
	CDependentReadObject	cObject;
	BOOL					bExists;
	int						iIndex;
	CDependentReadPointer*	pcPointer;

	cObject.Init(pcHeader);

	bExists = mcObjects.FindInSorted(&cObject, &CompareDependentReadObject, &iIndex);
	if (!bExists)
	{
		mcObjects.InsertAt(&cObject, iIndex);
	}
	else
	{
		cObject.Kill();
	}

	pcPointer = mcPointers.Add();
	pcPointer->Init(ppcObjectPtr, pcHeader->moi);
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


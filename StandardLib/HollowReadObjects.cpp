#include "Objects.h"
#include "IndexGenerator.h"
#include "HollowReadObjects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowReadObjects::Init(CIndexGenerator* pcIndexGenerator)
{
	mcObjects.Init(128);
	mcPointers.Init(512);
	miGetIndex = 0;
	mpcIndexGenerator = pcIndexGenerator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowReadObjects::Kill(void)
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
void CHollowReadObjects::Add(CPointerHeader* pcHeader, CBaseObject** ppcObjectPtr, CBaseObject* pcContaining)
{
	CDependentReadObject	cDependent;
	CDependentReadObject*	pcExistingInFile;
	BOOL					bExists;
	int						iIndex;
	CDependentReadPointer*	pcPointer;
	OIndex					oiNew;
	CPointerObject			pExisitingInDatabase;

	if (pcHeader->IsNamed())
	{
		return;
	}

	cDependent.Init(pcHeader);

	bExists = mcObjects.FindInSorted(&cDependent, &CompareDependentReadObject, &iIndex);
	if (!bExists)
	{
		oiNew = mpcIndexGenerator->PopIndex();
		cDependent.SetNewIndex(oiNew);
		mcObjects.InsertAt(&cDependent, iIndex);
	}
	else
	{
		pcExistingInFile = mcObjects.Get(iIndex);
		oiNew = pcExistingInFile->GetNewIndex();
		cDependent.Kill();
	}

	pcPointer = mcPointers.Add();
	pcPointer->Init(ppcObjectPtr, pcContaining, oiNew);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDependentReadObject* CHollowReadObjects::GetUnread(void)
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
void CHollowReadObjects::SetInitialIndex(OIndex oi)
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
BOOL CHollowReadObjects::Mark(OIndex oi)
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
int CHollowReadObjects::NumPointers(void)
{
	return mcPointers.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDependentReadPointer* CHollowReadObjects::GetPointer(int iIndex)
{
	return mcPointers.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowReadObjects::NumObjects(void)
{
	return mcObjects.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDependentReadObject* CHollowReadObjects::GetObject(int iIndex)
{
	return mcObjects.Get(iIndex);
}


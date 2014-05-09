#include "BaseLib/Log.h"
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
	mcReadObjects.Init(128);
	mcPointers.Init(512);
	mcIndexRemap.Init(128);
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

	mcIndexRemap.Kill();
	mcPointers.Kill();

	for(i = 0; i < mcReadObjects.NumElements(); i++)
	{
		pcDependent = mcReadObjects.Get(i);
		pcDependent->Kill();
	}
	mcReadObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::Add(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, unsigned short iNumEmbedded, unsigned short iEmbeddedIndex)
{
	CDependentReadObject	cDependent;
	CDependentReadObject*	pcExistingInFile;
	BOOL					bOiExistsInDependents;
	int						iIndex;
	CDependentReadPointer*	pcPointer;
	CPointer				pExisitingInDatabase;
	BOOL					bNameExistsInDatabase;

	cDependent.Init(pcHeader);

	bOiExistsInDependents = mcReadObjects.FindInSorted(&cDependent, &CompareDependentReadObject, &iIndex);
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

		mcReadObjects.InsertAt(&cDependent, iIndex);
	}
	else
	{
		pcExistingInFile = mcReadObjects.Get(iIndex);
		cDependent.Kill();
	}

	pcPointer = mcPointers.Add();
	pcPointer->Init(ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated, pcHeader->moi, iNumEmbedded, iEmbeddedIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDependentReadObject* CDependentReadObjects::GetUnread(void)
{
	int						iOldIndex;
	CDependentReadObject*	psObject;

	if (mcReadObjects.NumElements() == 0)
	{
		return NULL;
	}

	iOldIndex = miGetIndex;
	for (;;)
	{
		if (miGetIndex >= mcReadObjects.NumElements()-1)
		{
			miGetIndex = 0;
		}
		else
		{
			miGetIndex++;
		}

		psObject = mcReadObjects.Get(miGetIndex);
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
BOOL CDependentReadObjects::Mark(OIndex oi)
{
	CDependentReadObject*	pcDependent;

	pcDependent = GetObject(oi);
	if (!pcDependent)
	{
		return FALSE;
	}

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

	bResult = mcReadObjects.FindInSorted(&cObject, &CompareDependentReadObject, &iIndex);
	if (!bResult)
	{
		return NULL;
	}
	pcDependent = mcReadObjects.Get(iIndex);
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
	return mcReadObjects.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CDependentReadObjects::GetNewIndexFromOld(OIndex oiOld)
{
	int				i;
	CIndexNewOld*	pcRemap;

	for (i = 0; i < mcIndexRemap.NumElements(); i++)
	{
		pcRemap = mcIndexRemap.Get(i);
		if (pcRemap->moiOld == oiOld)
		{
			return pcRemap->moiNew;
		}
	}
	return INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::AddIndexRemap(OIndex oiNew, OIndex oiOld)
{
	CIndexNewOld*	pcNewOld;

	pcNewOld = mcIndexRemap.Add();
	pcNewOld->Init(oiNew, oiOld);
}


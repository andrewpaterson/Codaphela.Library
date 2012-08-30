#include "Objects.h"
#include "IndexGenerator.h"
#include "DependentReadObjects.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDependentReadObjects::Init(CIndexGenerator* pcIndexGenerator)
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
	CDependentReadObject	cDependent;
	CDependentReadObject*	pcExistingInFile;
	BOOL					bExists;
	int						iIndex;
	CDependentReadPointer*	pcPointer;
	OIndex					oiNew;
	CPointerObject			pExisitingInDatabase;

	cDependent.Init(pcHeader);

	bExists = mcObjects.FindInSorted(&cDependent, &CompareDependentReadObject, &iIndex);
	if (!bExists)
	{
		if (pcHeader->IsNamed())
		{
			//Well fuck.  If you have to talk to objects then what was the point of index generator?
			//Also you shouldn't be talking to gcObjects here.
			pExisitingInDatabase = gcObjects.Get(pcHeader->mszObjectName.Text());
			if (pExisitingInDatabase.IsNotNull())
			{
				oiNew = pExisitingInDatabase->GetOI();
				cDependent.SetExisting();
			}
			else
			{
				oiNew = mpcIndexGenerator->PopIndex();
			}
		}
		else
		{
			oiNew = mpcIndexGenerator->PopIndex();
		}

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
	pcPointer->Init(ppcObjectPtr, oiNew);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDependentReadObject* CDependentReadObjects::GetObject(int iIndex)
{
	return mcObjects.Get(iIndex);
}


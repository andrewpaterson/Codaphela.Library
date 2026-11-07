#include "BaseLib/Logger.h"
#include "BaseObject.h"
#include "Objects.h"
#include "KillOrderCalculator.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CKillOrderCalculator::Init(CObjects* pcObjects, CArrayBaseObjectPtr* papcKilled)
{
	mpcObjects = pcObjects;
	mpapcKilled = papcKilled;
	mapcKilledNesObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CKillOrderCalculator::Kill(void)
{
	mapcKilledNesObjects.Kill();
	mpapcKilled = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CKillOrderCalculator::Calculate(void)
{
	size			i;
	size			uiNumElements;
	size			uiKept;
	CBaseObject*	pcObject;

	//Both keep the order they were found in.
	uiKept = 0;
	uiNumElements = mpapcKilled->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = mpapcKilled->GetPtr(i);
		if (pcObject->IsNesPointedTo())
		{
			mapcKilledNesObjects.Add(pcObject);
		}
		else
		{
			*mpapcKilled->Get(uiKept) = pcObject;
			uiKept++;
		}
	}
	mpapcKilled->FakeSetUsedElements(uiKept);

	OrderNesObjects();
	return &mapcKilledNesObjects;
}


//////////////////////////////////////////////////////////////////////////
//
// Orders the Nes pointed to objects so each is freed after the objects
// freed with it that hold Nes pointers to it (a chain: a component needs
// its panel, which needs its window).  Of those that can go next the
// first found goes.  Objects holding Nes pointers to each other in a cycle
// can't be ordered; they are freed in the order found.
//
//////////////////////////////////////////////////////////////////////////
void CKillOrderCalculator::OrderNesObjects(void)
{
	CArrayBaseObjectPtr		apcWaiting;
	CBaseObject*			pcObject;
	size					i;
	size					uiNumWaiting;
	uint32					uiWaiting;
	bool					bFound;

	uiNumWaiting = mapcKilledNesObjects.NumElements();
	if (uiNumWaiting < 2)
	{
		return;
	}

	//The objects not ordered yet are stamped.
	uiWaiting = mpcObjects->StartWalk() | WALK_STATE_TOUCHED;
	apcWaiting.Init();
	apcWaiting.InsertArrayAfterEnd(&mapcKilledNesObjects);
	for (i = 0; i < uiNumWaiting; i++)
	{
		apcWaiting.GetPtr(i)->SetWalkStamp(uiWaiting);
	}

	mapcKilledNesObjects.ReInit();
	while (apcWaiting.NumElements() > 0)
	{
		bFound = false;
		uiNumWaiting = apcWaiting.NumElements();
		for (i = 0; i < uiNumWaiting; i++)
		{
			pcObject = apcWaiting.GetPtr(i);
			if (!IsNesPointedToFromWaiting(pcObject, uiWaiting))
			{
				mapcKilledNesObjects.Add(pcObject);
				pcObject->SetWalkStamp(0);
				apcWaiting.RemoveAt(i, true);
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			gcLogger.Error2(__METHOD__, " Objects freed together hold Nes pointers to each other in a cycle, starting with {", ObjectToString(apcWaiting.GetPtr(0)), "}.  They are freed in the order found.", NULL);
			mapcKilledNesObjects.InsertArrayAfterEnd(&apcWaiting);
			break;
		}
	}
	apcWaiting.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object still waiting to be ordered (other than pcObject
// itself) holds a Nes pointer to pcObject or an object embedded in it.
//
//////////////////////////////////////////////////////////////////////////
bool CKillOrderCalculator::IsNesPointedToFromWaiting(CBaseObject* pcObject, uint32 uiWaiting)
{
	CArrayEmbeddedObjectPtrE8	apcHolders;
	CBaseObject*				pcHolder;
	size						i;
	size						uiNumHolders;
	bool						bWaiting;

	bWaiting = false;
	apcHolders.Init();
	pcObject->GetNesPointerToThis(&apcHolders);
	uiNumHolders = apcHolders.NumElements();
	for (i = 0; i < uiNumHolders; i++)
	{
		pcHolder = apcHolders.GetPtr(i)->GetEmbeddingContainer();
		if ((pcHolder != pcObject) && (pcHolder->GetWalkStamp() == uiWaiting))
		{
			bWaiting = true;
			break;
		}
	}
	apcHolders.Kill();
	return bWaiting;
}

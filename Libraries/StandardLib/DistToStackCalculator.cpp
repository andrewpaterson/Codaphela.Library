#include "BaseObject.h"
#include "DistToStackCalculator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::Calculate(CBaseObject* pcFromChanged, CDistCalculatorParameters* pcParameters)
{
	pcFromChanged->CollectAndClearInvalidDistToRootObjects(pcParameters);
	ResetObjectsToUnattachedDistToRoot(pcParameters);
	CalculateFromTouched(pcParameters);
	pcParameters->ClearTouchedFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::CalculateFromTouched(CDistCalculatorParameters* pcParameters)
{
	size	iNumWithStackPointers;

	iNumWithStackPointers = CollectDetachedAndSetDistToStackZero(pcParameters);

	if (iNumWithStackPointers == 0)
	{
		//Kill them all!
		pcParameters->CopyRootDetachedToCompletelyDetached();
	}
	else if (iNumWithStackPointers == pcParameters->NumDetachedFromRoot())
	{
		//Kill none of them.
	}
	else
	{
		InitialiseCompletelyDetached(pcParameters);
		UpdateDistToStackForAllObjects(pcParameters);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::InitialiseCompletelyDetached(CDistCalculatorParameters* pcParameters)
{
	size			i;
	size			iNumDetached;
	CBaseObject*	pcBaseObject;

	iNumDetached = pcParameters->NumDetachedFromRoot();
	for (i = 0; i < iNumDetached; i++)
	{
		pcBaseObject = pcParameters->GetDetachedFromRoot(i);
		if (pcBaseObject->GetDistToStack() == UNKNOWN_DIST_TO_STACK)
		{
			//Copy everything with no stack dist into completely detached.  
			//They'll be moved out as the stack dists are updated.
			pcParameters->AddCompletelyDetached(pcBaseObject);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDistToStackCalculator::UpdateDistToStackForObjectsWithFromStackDist(CDistCalculatorParameters* pcParameters, CArrayBaseObjectPtr* papcUpdated)
{
	size				i;
	size				iNumDetached;
	CBaseObject*		pcBaseObject;
	int					iClosestFrom;
	CEmbeddedObject*	pcClosest;
	int					iNumUpdated;

	iNumDetached = pcParameters->NumCompletelyDetached();
	iNumUpdated = 0;
	for (i = iNumDetached; i > 0; i--)
	{
		pcBaseObject = pcParameters->GetCompletelyDetached(i - 1);
		pcClosest = pcBaseObject->GetClosestHeapFromToStack();
		if (pcClosest != NULL)
		{
			iNumUpdated++;
			iClosestFrom = pcClosest->GetDistToStack();
			pcBaseObject->SetDistToStack(iClosestFrom + 1);
			pcParameters->RemoveCompletelyDetached(i - 1);
			papcUpdated->Add(pcBaseObject);
		}
	}

	return iNumUpdated;
}


//////////////////////////////////////////////////////////////////////////
//
// Finds the completely detached objects that can still be reached from
// the stack.  First those pointed to from something with a distance to the
// stack, then what they point to, breadth first.  (Going over the
// detached objects again and again until none changed took a pass per
// object along a chain.)
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::UpdateDistToStackForAllObjects(CDistCalculatorParameters* pcParameters)
{
	CArrayBaseObjectPtr			apcReached;
	CArrayEmbeddedObjectPtr		apcTos;
	CBaseObject*				pcReached;
	CBaseObject*				pcTo;
	size						uiFirst;
	size						i;
	size						uiNumTos;
	int							iDistToStack;

	apcReached.Init();
	UpdateDistToStackForObjectsWithFromStackDist(pcParameters, &apcReached);

	for (uiFirst = 0; uiFirst < apcReached.NumElements(); uiFirst++)
	{
		pcReached = apcReached.GetPtr(uiFirst);
		iDistToStack = pcReached->GetDistToStack();

		apcTos.Init();
		pcReached->GetPointerTos(&apcTos);
		uiNumTos = apcTos.NumElements();
		for (i = 0; i < uiNumTos; i++)
		{
			pcTo = apcTos.GetPtr(i)->GetEmbeddingContainer();

			//Only objects this walk found detached from the root (the others are attached, or being freed).
			if ((pcTo->GetFlags() & OBJECT_FLAGS_DIST_FROM_WALKED) && (pcTo->GetDistToStack() == UNKNOWN_DIST_TO_STACK))
			{
				pcTo->SetDistToStack(iDistToStack + 1);
				apcReached.Add(pcTo);
			}
		}
		apcTos.Kill();
	}
	apcReached.Kill();

	for (i = pcParameters->NumCompletelyDetached(); i > 0; i--)
	{
		if (pcParameters->GetCompletelyDetached(i - 1)->GetDistToStack() != UNKNOWN_DIST_TO_STACK)
		{
			pcParameters->RemoveCompletelyDetached(i - 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistToStackCalculator::CollectDetachedAndSetDistToStackZero(CDistCalculatorParameters* pcParameters)
{
	size			i;
	CBaseObject*	pcBaseObject;
	size			iNumWithStackPointers;

	iNumWithStackPointers = 0;
	for (i = 0; i < pcParameters->NumTouched(); i++)
	{
		pcBaseObject = pcParameters->GetTouched(i);
		//I'm not sure if this is the right way to detect that object was declared on the stack.
		//And if on the stack it should not (ultimately) be attempted to be freed from Objects.
		if (pcBaseObject->IsAllocatedInObjects())
		{
			iNumWithStackPointers += pcBaseObject->CollectDetachedAndSetDistToStackZero(pcParameters);
		}
	}

	return iNumWithStackPointers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::ResetObjectsToUnknownDistToStack(CDistCalculatorParameters* pcParameters)
{
	size			i;
	size			iNumDetached;
	CBaseObject*	pcBaseObject;

	iNumDetached = pcParameters->NumDetachedFromRoot();
	for (i = 0; i < iNumDetached; i++)
	{
		pcBaseObject = pcParameters->GetDetachedFromRoot(i);
		if (pcBaseObject->IsAllocatedInObjects())
		{
			pcBaseObject->SetDistToStack(UNKNOWN_DIST_TO_STACK);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::ResetObjectsToUnattachedDistToRoot(CDistCalculatorParameters* pcParameters)
{
	size			i;
	size			iNumTouched;
	CBaseObject*	pcBaseObject;

	iNumTouched = pcParameters->NumTouched();

	for (i = 0; i < iNumTouched; i++)
	{
		pcBaseObject = pcParameters->GetTouched(i);
		if (pcBaseObject->GetDistToRoot() == CLEARED_DIST_TO_ROOT)
		{
			pcBaseObject->SetDistToRoot(UNATTACHED_DIST_TO_ROOT);
		}
	}
}


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
	int		iNumWithStackPointers;

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
	int				i;
	int				iNumDetached;
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
int CDistToStackCalculator::UpdateDistToStackForObjectsWithFromStackDist(CDistCalculatorParameters* pcParameters)
{
	int				i;
	int				iNumDetached;
	CBaseObject*	pcBaseObject;
	int				iClosestFrom;
	CBaseObject*	pcClosest;
	int				iNumUpdated;

	iNumDetached = pcParameters->NumCompletelyDetached();
	iNumUpdated = 0;
	for (i = iNumDetached-1; i >= 0; i--)
	{
		pcBaseObject = pcParameters->GetCompletelyDetached(i);
		pcClosest = pcBaseObject->GetClosestFromToStack();
		if (pcClosest != NULL)
		{
			iNumUpdated++;
			iClosestFrom = pcClosest->GetDistToStack();
			pcBaseObject->SetDistToStack(iClosestFrom+1);
			pcParameters->RemoveCompletelyDetached(i);
		}
	}

	return iNumUpdated;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::UpdateDistToStackForAllObjects(CDistCalculatorParameters* pcParameters)
{
	int iNumUpdated;

	iNumUpdated = -1;
	while (iNumUpdated != 0)
	{
		iNumUpdated = UpdateDistToStackForObjectsWithFromStackDist(pcParameters);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDistToStackCalculator::CollectDetachedAndSetDistToStackZero(CDistCalculatorParameters* pcParameters)
{
	int				i;
	CBaseObject*	pcBaseObject;
	int				iNumWithStackPointers;

	iNumWithStackPointers = 0;
	for (i = 0; i < pcParameters->NumTouched(); i++)
	{
		pcBaseObject = pcParameters->GetTouched(i);
		iNumWithStackPointers += pcBaseObject->CollectDetachedFroms(pcParameters);
	}

	return iNumWithStackPointers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::ResetObjectsToUnknownDistToStack(CDistCalculatorParameters* pcParameters)
{
	int				i;
	int				iNumDetached;
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
	int				i;
	int				iNumTouched;
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


#include "BaseObject.h"
#include "DistToStackCalculator.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::Kill(void)
{
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
		ResetObjectsToUnknownDistToStack(pcParameters);
	}
	else
	{
		InitialiseCompletelyDetached(pcParameters);
		UpdateDistToStackForAllObjects(pcParameters);
		ResetObjectsToUnknownDistToStack(pcParameters);
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
	int				iNumTouched;
	CBaseObject*	pcBaseObject;
	int				iNumWithStackPointers;
	int				iDistToRoot;

	iNumWithStackPointers = 0;
	iNumTouched = pcParameters->NumTouched();
	for (i = 0; i < iNumTouched; i++)
	{
		pcBaseObject = pcParameters->GetTouched(i);
		iDistToRoot = pcBaseObject->GetDistToRoot();
		if (iDistToRoot == UNATTACHED_DIST_TO_ROOT)
		{
			pcParameters->AddDetachedFromRoot(pcBaseObject);
			if (pcBaseObject->HasStackPointers())
			{
				pcBaseObject->SetDistToStack(0);
				iNumWithStackPointers++;
			}
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
	int				i;
	int				iNumDetached;
	CBaseObject*	pcBaseObject;

	iNumDetached = pcParameters->NumDetachedFromRoot();
	for (i = 0; i < iNumDetached; i++)
	{
		pcBaseObject = pcParameters->GetDetachedFromRoot(i);
		pcBaseObject->SetDistToStack(UNKNOWN_DIST_TO_STACK);
	}
}




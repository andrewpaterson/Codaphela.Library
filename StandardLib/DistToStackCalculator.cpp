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
void CDistToStackCalculator::Calculate(CDistDetachedFroms* pcDetached)
{
	int		iNumWithStackPointers;

	iNumWithStackPointers = UpdateDistToStackForObjectsWithStackPointers(pcDetached);
	if (iNumWithStackPointers == 0)
	{
		//Kill them all!
		pcDetached->CopyRootDetachedToCompletelyDetached();
	}
	else if (iNumWithStackPointers == pcDetached->NumDetachedFromRoot())
	{
		//Kill none of them.
		ResetObjectsToUnknownDistToStack(pcDetached);
	}
	else
	{
		InitialiseCompletelyDetached(pcDetached);
		UpdateDistToStackForAllObjects(pcDetached);
		ResetObjectsToUnknownDistToStack(pcDetached);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::InitialiseCompletelyDetached(CDistDetachedFroms* pcDetached)
{
	int				i;
	int				iNumDetached;
	CBaseObject*	pcBaseObject;

	iNumDetached = pcDetached->NumDetachedFromRoot();
	for (i = 0; i < iNumDetached; i++)
	{
		pcBaseObject = pcDetached->GetDetachedFromRoot(i);
		if (pcBaseObject->GetDistToStack() == UNKNOWN_DIST_TO_STACK)
		{
			pcDetached->AddCompletelyDetached(pcBaseObject);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDistToStackCalculator::UpdateDistToStackForObjectsWithFromStackDist(CDistDetachedFroms* pcDetached)
{
	int				i;
	int				iNumDetached;
	CBaseObject*	pcBaseObject;
	int				iClosestFrom;
	CBaseObject*	pcClosest;
	int				iNumUpdated;

	iNumDetached = pcDetached->NumCompletelyDetached();
	iNumUpdated = 0;
	for (i = iNumDetached-1; i >= 0; i--)
	{
		pcBaseObject = pcDetached->GetCompletelyDetached(i);
		pcClosest = pcBaseObject->GetClosestFromToStack();
		if (pcClosest != NULL)
		{
			iNumUpdated++;
			iClosestFrom = pcClosest->GetDistToStack();
			pcBaseObject->SetDistToStack(iClosestFrom+1);
			pcDetached->RemoveCompletelyDetached(i);
		}
	}

	return iNumUpdated;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::UpdateDistToStackForAllObjects(CDistDetachedFroms* pcDetached)
{
	int iNumUpdated;

	iNumUpdated = -1;
	while (iNumUpdated != 0)
	{
		iNumUpdated = UpdateDistToStackForObjectsWithFromStackDist(pcDetached);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDistToStackCalculator::UpdateDistToStackForObjectsWithStackPointers(CDistDetachedFroms* pcDetached)
{
	int				i;
	int				iNumDetached;
	CBaseObject*	pcBaseObject;
	int				iNumWithStackPointers;

	iNumWithStackPointers = 0;
	iNumDetached = pcDetached->NumDetachedFromRoot();
	for (i = 0; i < iNumDetached; i++)
	{
		pcBaseObject = pcDetached->GetDetachedFromRoot(i);
		if (pcBaseObject->HasStackPointers())
		{
			pcBaseObject->SetDistToStack(0);
			iNumWithStackPointers++;
		}
	}

	return iNumWithStackPointers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::ResetObjectsToUnknownDistToStack(CDistDetachedFroms* pcDetached)
{
	int				i;
	int				iNumDetached;
	CBaseObject*	pcBaseObject;

	iNumDetached = pcDetached->NumDetachedFromRoot();
	for (i = 0; i < iNumDetached; i++)
	{
		pcBaseObject = pcDetached->GetDetachedFromRoot(i);
		pcBaseObject->SetDistToStack(UNKNOWN_DIST_TO_STACK);
	}
}


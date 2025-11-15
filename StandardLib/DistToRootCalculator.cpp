#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseObject.h"
#include "DistToRootCalculator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::Calculate(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters)
{
	int					i;
	int					iNumTouched;
	CBaseObject*		pcTouched;
	CEmbeddedObject*	pcClosestFrom;

	CollectAndClearInvalidRootDistances(pcObject, pcParameters);

	iNumTouched = pcParameters->NumTouched();
	for (i = 0; i < iNumTouched; i++)
	{
		pcTouched = pcParameters->GetTouched(i);
		pcClosestFrom = pcTouched->GetClosestFromToRoot();
		if (pcClosestFrom)
		{
			pcParameters->AddExpectedDist(pcTouched, pcClosestFrom->GetDistToRoot()+1);
		}
	}

	UpdateAttachedTosDistToRoot(pcParameters);

	ResetObjectsToUnattachedDistToRoot(pcParameters);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::CollectAndClearInvalidRootDistances(CBaseObject* pcFromChanged, CDistCalculatorParameters* pcParameters)
{
	pcFromChanged->CollectAndClearInvalidDistToRootObjects(pcParameters);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::ResetObjectsToUnattachedDistToRoot(CDistCalculatorParameters* pcParameters)
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::ValidateExpectedDistStartingObjects(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters)
{
	if (pcParameters->NumExpectedDists() == 0) 
	{
		gcLogger.Error2(__METHOD__, " Could not find any starting objects for ", ObjectToString(pcObject, false), NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::UpdateAttachedTosDistToRoot(CDistCalculatorParameters* pcParameters)
{
	SDistToRoot*	psLowestDistToRoot;
	CBaseObject*	pcObject;
	int				iExpectedDist;

	psLowestDistToRoot = pcParameters->GetLowestExpectedDist();
	while (psLowestDistToRoot)
	{
		pcObject = psLowestDistToRoot->pcObject;
		iExpectedDist = psLowestDistToRoot->iExpectedDist;

		pcParameters->RemoveExpectedDist(psLowestDistToRoot);
		pcObject->UpdateAttachedTosDistToRoot(pcParameters);

		psLowestDistToRoot = pcParameters->GetLowestExpectedDist();
	}
}


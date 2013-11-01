#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseObject.h"
#include "DistToRootCalculator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::Init(void)
{
	mpcFromChanged = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::Kill(void)
{
	mpcFromChanged = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::Calculate(void)
{
	CDistCalculatorParameters	cDistParameters;

	cDistParameters.Init();

	Calculate(&cDistParameters);

	cDistParameters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::Calculate(CDistCalculatorParameters* pcParameters)
{
	CollectAndClearInvalidRootDistances(mpcFromChanged, pcParameters);
	
	int				i;
	int				iNumTouched;
	CBaseObject*	pcTouched;
	CBaseObject*	pcClosestFrom;

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

	ClearTouchedFlagsAndDetach(pcParameters);
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
void CDistToRootCalculator::ClearTouchedFlagsAndDetach(CDistCalculatorParameters* pcParameters)
{
	int				i;
	int				iNumTouched;
	CBaseObject*	pcBaseObject;

	iNumTouched = pcParameters->NumTouched();

	for (i = 0; i < iNumTouched; i++)
	{
		pcBaseObject = pcParameters->GetTouched(i);
		pcBaseObject->ClearDistTouchedFlags();
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
		CChars	sz;

		sz.Init();
		pcObject->PrintObject(&sz);
		gcLogger.Error2(__METHOD__, " Could not find any starting objects for ", sz.Text(), NULL);
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::AddFromChanged(CBaseObject* pcObject)
{
	mpcFromChanged = pcObject;
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


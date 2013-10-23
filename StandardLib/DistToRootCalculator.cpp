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
void CDistToRootCalculator::AddFromChanged(CBaseObject* pcObject)
{
	mpcFromChanged = pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::Calculate(void)
{
	CDistToRootEffectedFroms	cEffectedFroms;

	cEffectedFroms.Init();

	Calculate(&cEffectedFroms);

	cEffectedFroms.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::Calculate(CDistToRootEffectedFroms* pcEffectedFroms)
{
	mpcFromChanged->ClearDistToRootToValidDist(NULL, pcEffectedFroms);
	UpdateTosDistToRoot(pcEffectedFroms);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::UpdateTosDistToRoot(CDistToRootEffectedFroms* pcEffectedFroms)
{
	SDistToRoot*			psLowestDistToRoot;
	SDistToRoot*			psSecondLowestDistToRoot;
	int						iStopDist;
	CBaseObject*			pcObject;
	int						iExpectedDist;
	int						i;
	CArrayBaseObjectPtr*	papcLowestFroms;

	pcEffectedFroms->MarkLowestFroms();
	
	psLowestDistToRoot = pcEffectedFroms->GetLowest();	
	while (psLowestDistToRoot)
	{
		psSecondLowestDistToRoot = pcEffectedFroms->GetSecondLowest(psLowestDistToRoot);
		if (psSecondLowestDistToRoot)
		{
			iStopDist = psSecondLowestDistToRoot->iExpectedDist+1;
		}
		else
		{
			iStopDist = MAX_DIST_TO_ROOT;
		}

		iExpectedDist = psLowestDistToRoot->iExpectedDist;
		pcObject = psLowestDistToRoot->pcObject;
		pcEffectedFroms->Remove(psLowestDistToRoot);

		pcObject->UpdateTosDistToRoot(pcEffectedFroms, iStopDist, iExpectedDist);

		psLowestDistToRoot = pcEffectedFroms->GetLowest();	
	}

	papcLowestFroms = pcEffectedFroms->GetLowestFroms();
	for (i = 0; i < papcLowestFroms->NumElements(); i++)
	{
		pcObject = *papcLowestFroms->Get(i);
		pcObject->DoneUpdateTosDistToRoot();
	}
}


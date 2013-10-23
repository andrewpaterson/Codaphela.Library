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
	CDistDetachedFroms			cDetached;

	cEffectedFroms.Init();
	cDetached.Init();

	Calculate(&cEffectedFroms, &cDetached);

	cDetached.Kill();
	cEffectedFroms.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::Calculate(CDistToRootEffectedFroms* pcEffectedFroms, CDistDetachedFroms* pcDetached)
{
	mpcFromChanged->ClearDistToRootToValidDist(NULL, pcEffectedFroms);
	UpdateTosDistToRoot(pcEffectedFroms);
	ClearTosUpdatedToRootFlag(pcEffectedFroms);

	if (mpcFromChanged->GetDistToRoot() == CLEARED_DIST_TO_ROOT)
	{
		mpcFromChanged->UpdateTosDetached(pcDetached);
	}
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::ClearTosUpdatedToRootFlag(CDistToRootEffectedFroms* pcEffectedFroms)
{
	CArrayBaseObjectPtr*	papcLowestFroms;
	CBaseObject*			pcObject;
	int						i;

	papcLowestFroms = pcEffectedFroms->GetLowestFroms();
	for (i = 0; i < papcLowestFroms->NumElements(); i++)
	{
		pcObject = *papcLowestFroms->Get(i);
		pcObject->ClearTosUpdatedToRootFlag();
	}
}


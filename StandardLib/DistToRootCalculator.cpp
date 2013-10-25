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
	//Check if the "FromChanged" object has any froms pointing to it that can still find the Root object.
	//If they can add those froms pointing to it objects to an array of objects to start from.
	mpcFromChanged->ClearDistToRootToValidDist(NULL, pcEffectedFroms);

	//Copy the starting from objects into an array so their flags can be fixed later.
	pcEffectedFroms->MarkLowestFroms();

	//For all the starting objects walk the 'tos' and update their 'to' graphs distances to the Root object.
	UpdateTosDistToRoot(pcEffectedFroms);

	//If the changed from can no longer find the root object then it is detached and it's tos must be detached also,
	//Unless another from pointig to the 'detached' to can find the Root object.
	if (mpcFromChanged->GetDistToRoot() == CLEARED_DIST_TO_ROOT)
	{
		mpcFromChanged->UpdateTosDetached(pcDetached, pcEffectedFroms);

		pcEffectedFroms->MarkLowestFroms();

		UpdateTosDistToRoot(pcEffectedFroms);
	}

	ClearTosUpdatedToRootFlag(pcEffectedFroms);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::UpdateTosDistToRoot(CDistToRootEffectedFroms* pcEffectedFroms)
{
	SDistToRoot*			psLowestDistToRoot;
	CBaseObject*			pcObject;
	int						iExpectedDist;

	psLowestDistToRoot = pcEffectedFroms->GetLowest();	
	while (psLowestDistToRoot)
	{
		iExpectedDist = psLowestDistToRoot->iExpectedDist;
		pcObject = psLowestDistToRoot->pcObject;
		pcEffectedFroms->Remove(psLowestDistToRoot);

		pcObject->UpdateTosDistToRoot(pcEffectedFroms, iExpectedDist);

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


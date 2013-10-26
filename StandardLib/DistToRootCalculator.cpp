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
void CDistToRootCalculator::Calculate(CDistDetachedFroms* pcDetached)
{
	CDistToRootEffectedFroms	cEffectedFroms;

	cEffectedFroms.Init();

	Calculate(&cEffectedFroms, pcDetached);

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

	//Cyclic dependencies will cause lowest pointers to be incorrectly added.  Remove them.
	RemoveDetachedLowest(pcEffectedFroms);

	//Copy the starting from objects into an array so their flags can be fixed later.
	pcEffectedFroms->MarkLowestFroms();

	//For all the starting objects walk the 'tos' and update their 'to' graphs distances to the Root object.
	UpdateTosDistToRoot(pcEffectedFroms);

	//If the changed from can no longer find the root object then it is detached and it's tos must be detached also,
	//Unless another from pointing to the 'detached' to can find the Root object.
	if (mpcFromChanged->GetDistToRoot() == CLEARED_DIST_TO_ROOT)
	{
		mpcFromChanged->UpdateTosDetached(pcDetached, pcEffectedFroms);

		pcEffectedFroms->MarkLowestFroms();

		UpdateTosDistToRoot(pcEffectedFroms);
	}

	pcEffectedFroms->AddChangedFromAsLowest(mpcFromChanged);
	ClearTosUpdatedTosFlags(pcEffectedFroms);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::RemoveDetachedLowest(CDistToRootEffectedFroms* pcEffectedFroms)
{
	int				i;
	CBaseObject*	pcBaseObject;

	for (i = pcEffectedFroms->NumElements()-1; i >= 0; i--)
	{
		pcBaseObject = pcEffectedFroms->Get(i)->pcObject;
		if (!pcBaseObject->CanFindRoot())
		{
			pcEffectedFroms->Remove(i);
		}
	}
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

	//This is probably really slow.
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
void CDistToRootCalculator::ClearTosUpdatedTosFlags(CDistToRootEffectedFroms* pcEffectedFroms)
{
	CArrayBaseObjectPtr*	papcLowestFroms;
	CBaseObject*			pcObject;
	int						i;

	papcLowestFroms = pcEffectedFroms->GetLowestFroms();
	for (i = 0; i < papcLowestFroms->NumElements(); i++)
	{
		pcObject = *papcLowestFroms->Get(i);
		pcObject->ClearTosUpdatedTosFlags();
	}
}


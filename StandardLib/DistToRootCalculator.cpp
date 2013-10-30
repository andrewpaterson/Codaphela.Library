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
	//These finder methods belong in a different calculator
	{
		//Check if the "FromChanged" object has any froms pointing to it that can still find the Root object.
		//If they can add those froms pointing to it objects to an array of objects to start from (macExpectedDists).
		//
		//Also check if the "FromChanged" object has any froms pointing to it that can still find the stack.
		//If they can add those froms pointing to it objects to an array of objects to start from (mapcUnattched).
		//
		//This method also has side effect of setting the objects dist to root to CLEARED_DIST_TO_ROOT.
		mpcFromChanged->CollectStartingObjectsAndSetClearedToRoot(NULL, pcParameters);
	}

	pcParameters->MarkExpectedDistLowestFroms();

	//Firstly deal with the case where objects pointed to by our potentially detached object can still find the root.
	UpdateAttachedTosDistToRoot(pcParameters);

	//If the changed from can no longer find the root object then it is detached and it's tos must be detached also,
	//Unless another from pointing to the 'detached' to can find the Root object.
	if (mpcFromChanged->GetDistToRoot() == CLEARED_DIST_TO_ROOT)
	{
		mpcFromChanged->UpdateTosDetached(pcParameters);  //This is a weird shitty method.

		pcParameters->MarkExpectedDistLowestFroms();
		UpdateAttachedTosDistToRoot(pcParameters);
	}

	pcParameters->MarkUnattachedLowestFroms();

	pcParameters->AddChangedFromAsLowest(mpcFromChanged);

	//This method adds additional unattached objects.  Which is bad.
	//Basically all this method does is set the DistToRoot to UNATTACHED_DIST_TO_ROOT rather than CLEARED_DIST_TO_ROOT.
	UpdateUnattachedTosDistToRoot(pcParameters);

	ClearTosFlagsFromLowest(pcParameters);
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
	SDistToRoot*			psLowestDistToRoot;
	CBaseObject*			pcObject;

	//This is probably really slow.
	psLowestDistToRoot = pcParameters->GetLowest();
	while (psLowestDistToRoot)
	{
		pcObject = psLowestDistToRoot->pcObject;
		pcParameters->RemoveExpectedDist(psLowestDistToRoot);
		pcObject->UpdateTosDistToRoot(pcParameters);

		psLowestDistToRoot = pcParameters->GetLowest();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::UpdateUnattachedTosDistToRoot(CDistCalculatorParameters* pcParameters)
{
	CBaseObject*			pcBaseObject;

	pcBaseObject = pcParameters->GetUnattached();
	while (pcBaseObject)
	{
		pcParameters->RemoveUnattached(pcBaseObject);
		pcBaseObject->UpdateTosUnattached(pcParameters);

		pcBaseObject = pcParameters->GetUnattached();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToRootCalculator::ClearTosFlagsFromLowest(CDistCalculatorParameters* pcParameters)
{
	CArrayBaseObjectPtr*	papcLowestFroms;
	CBaseObject*			pcObject;
	int						i;

	//Rather than doing this add every object touched into an array.  It's cheaper in the long run.
	papcLowestFroms = pcParameters->GetLowestFroms();
	for (i = 0; i < papcLowestFroms->NumElements(); i++)
	{
		pcObject = *papcLowestFroms->Get(i);
		pcObject->ClearTosFlagsFromLowest();
	}
}


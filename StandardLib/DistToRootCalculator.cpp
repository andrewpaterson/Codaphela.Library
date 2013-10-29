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
		//If they can add those froms pointing to it objects to an array of objects to start from.
		mpcFromChanged->CollectStartingObjects(NULL, pcParameters);

		//Cyclic dependencies will cause lowest pointers to be incorrectly added.  Remove them.
		RemoveDetachedLowest(pcParameters);
	}

	//Copy the starting from objects into an array so their flags can be fixed later.
	pcParameters->MarkExpectedDistLowestFroms();

	//For all the starting objects walk the 'tos' and update their 'to' graphs distances to the Root object.
	UpdateAttachedTosDistToRoot(pcParameters);

	//If the changed from can no longer find the root object then it is detached and it's tos must be detached also,
	//Unless another from pointing to the 'detached' to can find the Root object.
	if (mpcFromChanged->GetDistToRoot() == CLEARED_DIST_TO_ROOT)
	{
		mpcFromChanged->UpdateTosDetached(pcParameters);

		pcParameters->MarkExpectedDistLowestFroms();

		UpdateAttachedTosDistToRoot(pcParameters);
	}

	pcParameters->MarkUnattachedLowestFroms();

	pcParameters->AddChangedFromAsLowest(mpcFromChanged);

	//This method adds additional unattached objects.  Which is bad.
	UpdateUnattachedTosDistToRoot(pcParameters);

	ClearTosUpdatedTosFlags(pcParameters);
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
void CDistToRootCalculator::RemoveDetachedLowest(CDistCalculatorParameters* pcParameters)
{
	int				i;
	CBaseObject*	pcBaseObject;
	SDistToRoot*	psDistToRoot;
	int				iNumEffectedFroms;

	iNumEffectedFroms = pcParameters->NumExpectedDists();
	for (i = iNumEffectedFroms-1; i >= 0; i--)
	{
		psDistToRoot = pcParameters->GetExpectedDist(i);
		pcBaseObject = psDistToRoot->pcObject;
		if (!pcBaseObject->CanFindRoot())
		{
			pcParameters->RemoveExpectedDist(i);
		}
	}
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
void CDistToRootCalculator::ClearTosUpdatedTosFlags(CDistCalculatorParameters* pcParameters)
{
	CArrayBaseObjectPtr*	papcLowestFroms;
	CBaseObject*			pcObject;
	int						i;

	papcLowestFroms = pcParameters->GetLowestFroms();
	for (i = 0; i < papcLowestFroms->NumElements(); i++)
	{
		pcObject = *papcLowestFroms->Get(i);
		pcObject->ClearTosUpdatedTosFlags();
	}
}


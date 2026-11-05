#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseObject.h"
#include "Objects.h"
#include "DistCalculator.h"


//////////////////////////////////////////////////////////////////////////
//
// A heap from of pcFromRemoved went.  Clears the distances that relied on
// it, then gives the cleared objects their distances again from the froms
// that are left, closest first.  Those that can't find the root are left
// unattached.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::CalculateDistToRoot(CBaseObject* pcFromRemoved)
{
	size						i;
	size						uiNumTouched;
	CBaseObject*				pcTouched;
	CEmbeddedObject*			pcClosestFrom;
	CDistToRootExpectedDists	cExpectedDists;

	CollectAndClearInvalidDistToRoot(pcFromRemoved);

	cExpectedDists.Init();
	uiNumTouched = NumTouched();
	for (i = 0; i < uiNumTouched; i++)
	{
		pcTouched = GetTouched(i);
		pcClosestFrom = pcTouched->GetClosestFromToRoot();
		if (pcClosestFrom)
		{
			cExpectedDists.Add(pcTouched, pcClosestFrom->GetDistToRoot() + 1);
		}
	}

	UpdateAttachedTosDistToRoot(&cExpectedDists);
	cExpectedDists.Kill();

	ResetClearedToUnattached();
}


//////////////////////////////////////////////////////////////////////////
//
// Clears pcFromRemoved's distance to the root if it can't still be found
// from the root, and so on for what it points to, depth first.  The walk
// keeps its own stack of candidates: a recursive one overflowed the
// W65C816's stack on a chain a hundred objects long.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::CollectAndClearInvalidDistToRoot(CBaseObject* pcFromRemoved)
{
	CArrayDistToRootClearCandidates		acCandidates;
	SDistToRootClearCandidate			sCandidate;
	SDistToRootClearCandidate*			psCandidate;

	acCandidates.Init();
	psCandidate = acCandidates.Add();
	psCandidate->pcObject = pcFromRemoved;
	psCandidate->iClearedFromDistToRoot = MAX_DIST_TO_ROOT;
	while (acCandidates.Pop(&sCandidate))
	{
		ClearInvalidDistToRoot(sCandidate.pcObject, sCandidate.iClearedFromDistToRoot, &acCandidates);
	}
	acCandidates.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// iClearedFromDistToRoot is the distance the from that was just cleared
// had.  A from no closer to root than pcObject (or not attached to it at
// all) was never what it relied on, so pcObject need not be checked (a
// window that every component points to would otherwise be checked once
// for every cleared component).  MAX_DIST_TO_ROOT always checks: it is
// used for the object whose from was removed.  When pcObject is cleared,
// what it points to are added as candidates, to be checked in order.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::ClearInvalidDistToRoot(CBaseObject* pcObject, int iClearedFromDistToRoot, CArrayDistToRootClearCandidates* pacCandidates)
{
	int							iDistToRoot;
	CArrayEmbeddedObjectPtr*	papcTos;
	size						i;
	SDistToRootClearCandidate*	psCandidate;

	if (pcObject->IsEmbedded())
	{
		pcObject->FailNotExpectedToBeEmbedded(__METHOD__);
		return;
	}

	if (pcObject->IsRoot())
	{
		return;
	}

	//Objects in the batch being freed are gone already; touched ones are cleared already.
	if ((pcObject->GetFlags() & OBJECT_FLAGS_FREED) || (GetWalkState(pcObject) != 0))
	{
		return;
	}

	iDistToRoot = pcObject->GetDistToRoot();
	if ((iDistToRoot >= ROOT_DIST_TO_ROOT) && (iClearedFromDistToRoot != MAX_DIST_TO_ROOT))
	{
		if ((iClearedFromDistToRoot < ROOT_DIST_TO_ROOT) || (iDistToRoot <= iClearedFromDistToRoot))
		{
			return;
		}
	}

	if (HasCloserFrom(pcObject))
	{
		return;
	}

	AddTouched(pcObject);
	SetWalkState(pcObject, WALK_STATE_TOUCHED);
	pcObject->SetDistToRoot(CLEARED_DIST_TO_ROOT);

	//What it (and the objects embedded in it) points to, added last first so the first is checked first.
	papcTos = StartPointerTos();
	pcObject->GetPointerTos(papcTos);
	for (i = papcTos->NumElements(); i > 0; i--)
	{
		psCandidate = pacCandidates->Add();
		psCandidate->pcObject = papcTos->GetPtr(i - 1)->GetEmbeddingContainer();
		psCandidate->iClearedFromDistToRoot = iDistToRoot;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// While clearing, an object keeps its distance when a from is closer to
// root and not cleared.  That from is valid by the same rule: every
// attached object had a closer from before the change, and clearing an
// object rechecks everything it points to.  So no walk back to root is
// needed (a walk that scanned hubs, like a window, on every step).
//
//////////////////////////////////////////////////////////////////////////
bool CDistCalculator::HasCloserFrom(CBaseObject* pcObject)
{
	CEmbeddedObject*	pcClosestFrom;
	int					iDistToRoot;

	iDistToRoot = pcObject->GetDistToRoot();
	if (iDistToRoot <= ROOT_DIST_TO_ROOT)
	{
		return false;
	}

	pcClosestFrom = pcObject->GetClosestFromToRoot();
	if (pcClosestFrom == NULL)
	{
		return false;
	}
	return pcClosestFrom->GetDistToRoot() < iDistToRoot;
}


//////////////////////////////////////////////////////////////////////////
//
// The touched objects still cleared couldn't find the root.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::ResetClearedToUnattached(void)
{
	size			i;
	size			uiNumTouched;
	CBaseObject*	pcBaseObject;

	uiNumTouched = NumTouched();
	for (i = 0; i < uiNumTouched; i++)
	{
		pcBaseObject = GetTouched(i);
		if (pcBaseObject->GetDistToRoot() == CLEARED_DIST_TO_ROOT)
		{
			pcBaseObject->SetDistToRoot(UNATTACHED_DIST_TO_ROOT);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gives the objects with an expected distance their distances, closest
// first, so each gets its final distance the first time.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::UpdateAttachedTosDistToRoot(CDistToRootExpectedDists* pcExpectedDists)
{
	SDistToRoot*	psLowestDistToRoot;
	CBaseObject*	pcObject;

	psLowestDistToRoot = pcExpectedDists->GetLowest();
	while (psLowestDistToRoot)
	{
		pcObject = psLowestDistToRoot->pcObject;

		pcExpectedDists->Remove(psLowestDistToRoot);
		UpdateAttachedTosDistToRoot(pcObject, pcExpectedDists);

		psLowestDistToRoot = pcExpectedDists->GetLowest();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::UpdateAttachedTosDistToRoot(CBaseObject* pcObject, CDistToRootExpectedDists* pcExpectedDists)
{
	CEmbeddedObject*			pcClosestToRoot;
	int							iClosestToRoot;
	CArrayEmbeddedObjectPtr*	papcTos;
	size						i;
	size						uiNumTos;

	if (pcObject->IsEmbedded())
	{
		pcObject->FailNotExpectedToBeEmbedded(__METHOD__);
		return;
	}

	pcClosestToRoot = pcObject->GetClosestFromToRoot();
	iClosestToRoot = pcClosestToRoot->GetDistToRoot() + 1;

	AddTouched(pcObject);
	pcObject->SetDistToRoot(iClosestToRoot);

	//What it (and the objects embedded in it) points to can be one further.
	papcTos = StartPointerTos();
	pcObject->GetPointerTos(papcTos);
	uiNumTos = papcTos->NumElements();
	for (i = 0; i < uiNumTos; i++)
	{
		AddExpectedDistToRoot(papcTos->GetPtr(i), iClosestToRoot + 1, pcExpectedDists);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An object that still has a dist to root was not cleared (or has been
// given its distance back, closest first, so it is final) so already has
// a valid path to the root.  Only a shorter path changes it; clearing it
// would also clear the objects embedded in it that others are found from.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::AddExpectedDistToRoot(CEmbeddedObject* pcPointedTo, int iExpectedDist, CDistToRootExpectedDists* pcExpectedDists)
{
	CBaseObject*	pcPointedToContainer;
	int				iDistToRoot;

	if (pcPointedTo)
	{
		pcPointedToContainer = pcPointedTo->GetEmbeddingContainer();
		iDistToRoot = pcPointedToContainer->GetDistToRoot();
		if ((iDistToRoot >= ROOT_DIST_TO_ROOT) && (iDistToRoot <= iExpectedDist))
		{
			return;
		}

		pcPointedToContainer->SetDistToRoot(CLEARED_DIST_TO_ROOT);
		pcExpectedDists->Add(pcPointedToContainer, iExpectedDist);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A heap from iExpectedDistToRoot - 1 from the root was added to pcObject.
// It can only bring pcObject, and what it points to, closer.  pcObject's
// distance is already the best of its froms', so only a shorter one
// changes anything.  The objects that got closer are queued and give what
// they point to their distance in turn, breadth first, so each gets its
// final distance the first time it's reached.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::FromAdded(CBaseObject* pcObject, int iExpectedDistToRoot)
{
	CArrayBaseObjectPtr			apcCloser;
	CArrayEmbeddedObjectPtr		apcTos;
	CBaseObject*				pcCloser;
	CBaseObject*				pcTo;
	size						uiFirst;
	size						i;
	size						uiNumTos;
	int							iDistToRoot;

	if (pcObject->IsEmbedded())
	{
		pcObject->FailNotExpectedToBeEmbedded(__METHOD__);
		return;
	}

	if (iExpectedDistToRoot < ROOT_DIST_TO_ROOT)
	{
		gcLogger.Error2(__METHOD__, " Cannot expect dist to root [", IntToString(iExpectedDistToRoot), "]; only a from attached to the root gives one.", NULL);
		return;
	}

	iDistToRoot = pcObject->GetDistToRoot();
	if ((iDistToRoot >= ROOT_DIST_TO_ROOT) && (iDistToRoot <= iExpectedDistToRoot))
	{
		return;
	}

	pcObject->SetDistToRoot(iExpectedDistToRoot);

	apcCloser.Init();
	apcTos.Init();
	apcCloser.Add(pcObject);
	for (uiFirst = 0; uiFirst < apcCloser.NumElements(); uiFirst++)
	{
		pcCloser = apcCloser.GetPtr(uiFirst);
		iExpectedDistToRoot = pcCloser->GetDistToRoot() + 1;

		apcTos.FakeSetUsedElements(0);
		pcCloser->GetPointerTos(&apcTos);
		uiNumTos = apcTos.NumElements();
		for (i = 0; i < uiNumTos; i++)
		{
			pcTo = apcTos.GetPtr(i)->GetEmbeddingContainer();
			iDistToRoot = pcTo->GetDistToRoot();
			if ((iDistToRoot < ROOT_DIST_TO_ROOT) || (iDistToRoot > iExpectedDistToRoot))
			{
				pcTo->SetDistToRoot(iExpectedDistToRoot);
				apcCloser.Add(pcTo);
			}
		}
	}
	apcTos.Kill();
	apcCloser.Kill();
}


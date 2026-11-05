#include "BaseObject.h"
#include "Objects.h"
#include "DistCalculator.h"


//////////////////////////////////////////////////////////////////////////
//
// A stack from of pcFromRemoved went.  Clears the distances to the root
// that can't be found any more (an object loaded without the root may
// have none to clear), then finds what can't be reached at all.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::CalculateDistToStack(CBaseObject* pcFromRemoved)
{
	CollectAndClearInvalidDistToRoot(pcFromRemoved);
	ResetClearedToUnattached();
	CalculateFromTouched();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::CalculateFromTouched(void)
{
	size	uiNumWithStackPointers;

	uiNumWithStackPointers = CollectDetachedFromRoot();

	if (uiNumWithStackPointers == 0)
	{
		//Kill them all!
		CopyRootDetachedToCompletelyDetached();
	}
	else if (uiNumWithStackPointers == NumDetachedFromRoot())
	{
		//Kill none of them.
	}
	else
	{
		InitialiseCompletelyDetached();
		FindReachedFromStack();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Collects the touched objects, and the objects that point to them (and to
// them...), that are detached from the root.  Returns how many of them are
// pointed to from the stack.
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculator::CollectDetachedFromRoot(void)
{
	size					i;
	CBaseObject*			pcBaseObject;
	size					uiNumWithStackPointers;
	CArrayBaseObjectPtr		apcFroms;

	uiNumWithStackPointers = 0;
	apcFroms.Init();
	for (i = 0; i < NumTouched(); i++)
	{
		pcBaseObject = GetTouched(i);
		//I'm not sure if this is the right way to detect that object was declared on the stack.
		//And if on the stack it should not (ultimately) be attempted to be freed from Objects.
		if (pcBaseObject->IsAllocatedInObjects())
		{
			uiNumWithStackPointers += CollectDetachedFromRoot(pcBaseObject, &apcFroms);
		}
	}
	apcFroms.Kill();

	return uiNumWithStackPointers;
}


//////////////////////////////////////////////////////////////////////////
//
// Collects pcObject and the objects that point to it (and to them...) that
// are detached from the root, depth first.  Returns how many of them are
// pointed to from the stack.  The walk keeps its own stack (papcFroms,
// empty on entry and exit): a recursive one overflowed the W65C816's stack
// on a chain a hundred objects long.
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculator::CollectDetachedFromRoot(CBaseObject* pcObject, CArrayBaseObjectPtr* papcFroms)
{
	size	uiNumWithStackPointers;

	uiNumWithStackPointers = 0;
	papcFroms->Add(pcObject);
	while (papcFroms->NumElements() > 0)
	{
		pcObject = papcFroms->Pop();
		if (CollectDetached(pcObject, papcFroms))
		{
			uiNumWithStackPointers++;
		}
	}
	return uiNumWithStackPointers;
}


//////////////////////////////////////////////////////////////////////////
//
// Collects pcObject if it is detached and not yet walked, and adds the
// objects that point to it to papcFroms (reversed, so the first is walked
// first).  Returns whether it was collected and is pointed to from the
// stack.
//
//////////////////////////////////////////////////////////////////////////
bool CDistCalculator::CollectDetached(CBaseObject* pcObject, CArrayBaseObjectPtr* papcFroms)
{
	size			uiFirst;
	size			uiLast;
	CBaseObject*	pcSwap;
	bool			bHasStackFroms;
	uint32			uiState;

	if (pcObject->IsEmbedded())
	{
		pcObject->FailNotExpectedToBeEmbedded(__METHOD__);
		return false;
	}

	if (pcObject->GetDistToRoot() != UNATTACHED_DIST_TO_ROOT)
	{
		return false;
	}

	//Walked already, or in the batch being freed (gone already: neither collected nor walked through).
	uiState = GetWalkState(pcObject);
	if ((uiState >= WALK_STATE_DETACHED_WALKED) || (pcObject->GetFlags() & OBJECT_FLAGS_FREED))
	{
		return false;
	}

	AddDetachedFromRoot(pcObject);
	if (uiState == 0)
	{
		AddTouched(pcObject);
	}

	bHasStackFroms = pcObject->HasStackFroms();
	if (bHasStackFroms)
	{
		MarkReachedFromStack(pcObject);
	}
	else
	{
		SetWalkState(pcObject, WALK_STATE_DETACHED_WALKED);
	}

	uiFirst = papcFroms->NumElements();
	pcObject->AddHeapFromContainers(papcFroms);
	uiLast = papcFroms->NumElements();
	while (uiLast > uiFirst + 1)
	{
		uiLast--;
		pcSwap = papcFroms->GetPtr(uiFirst);
		*papcFroms->Get(uiFirst) = papcFroms->GetPtr(uiLast);
		*papcFroms->Get(uiLast) = pcSwap;
		uiFirst++;
	}
	return bHasStackFroms;
}


//////////////////////////////////////////////////////////////////////////
//
// A stack from of pcFromRemoved (unattached, with no stack froms left)
// went.  If it was known held from the stack and no older from known held
// holds it now (see CBaseObject::IsKnownHeldFromStack), it, and the newer
// objects that were known held through it, are cleared; then those with an
// older from still known held are known held again, and so on for the
// newer ones they point to.  Returns whether pcFromRemoved is known held:
// then nothing needs walking.
//
//////////////////////////////////////////////////////////////////////////
bool CDistCalculator::RecalculateKnownHeldFromStack(CBaseObject* pcFromRemoved)
{
	CArrayBaseObjectPtr		apcCandidates;
	CArrayBaseObjectPtr		apcCleared;
	CBaseObject*			pcObject;
	uint32					uiCleared;

	if (!pcFromRemoved->IsKnownHeldFromStack())
	{
		return false;
	}
	if (HasStackOrOlderKnownHeldFrom(pcFromRemoved))
	{
		return true;
	}

	//Clear what relied on it.
	uiCleared = gcObjects.StartWalk() | WALK_STATE_TOUCHED;
	apcCandidates.Init();
	apcCleared.Init();
	apcCandidates.Add(pcFromRemoved);
	while (apcCandidates.NumElements() > 0)
	{
		ClearKnownHeldFromStack(apcCandidates.Pop(), uiCleared, &apcCandidates, &apcCleared);
	}

	//Those still held by an older from known held are again, and then the newer ones they point to.
	while (apcCleared.NumElements() > 0)
	{
		pcObject = apcCleared.Pop();
		if ((pcObject->GetWalkStamp() == uiCleared) && HasStackOrOlderKnownHeldFrom(pcObject))
		{
			pcObject->SetWalkStamp(0);
			pcObject->SetKnownHeldFromStack();
			AddNewerTos(pcObject, &apcCleared);
		}
	}
	apcCandidates.Kill();
	apcCleared.Kill();

	return pcFromRemoved->IsKnownHeldFromStack();
}


//////////////////////////////////////////////////////////////////////////
//
// Clears pcObject's known held if nothing holds it any more, and adds the
// newer objects it points to (that may have relied on it) as candidates.
// As for distances to the root, a from that is cleared later rechecks what
// it points to, so an older from known held and not (yet) cleared is
// enough.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::ClearKnownHeldFromStack(CBaseObject* pcObject, uint32 uiCleared, CArrayBaseObjectPtr* papcCandidates, CArrayBaseObjectPtr* papcCleared)
{
	if (!pcObject->IsKnownHeldFromStack() || (pcObject->GetFlags() & OBJECT_FLAGS_FREED))
	{
		return;
	}
	if (HasStackOrOlderKnownHeldFrom(pcObject))
	{
		return;
	}

	pcObject->SetWalkStamp(uiCleared);
	pcObject->SetDistToRoot(UNATTACHED_DIST_TO_ROOT);
	papcCleared->Add(pcObject);
	AddNewerTos(pcObject, papcCandidates);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds what pcObject (and the objects embedded in it) points to that is
// newer than it: only those can be held through it.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::AddNewerTos(CBaseObject* pcObject, CArrayBaseObjectPtr* papcObjects)
{
	CArrayEmbeddedObjectPtr		apcTos;
	CBaseObject*				pcTo;
	size						i;
	size						uiNumTos;

	apcTos.Init();
	pcObject->GetPointerTos(&apcTos);
	uiNumTos = apcTos.NumElements();
	for (i = 0; i < uiNumTos; i++)
	{
		pcTo = apcTos.GetPtr(i)->GetEmbeddingContainer();
		if (pcObject->IsOlderThan(pcTo))
		{
			papcObjects->Add(pcTo);
		}
	}
	apcTos.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcObject is still known held: by a stack from, or by an older
// from known held.
//
//////////////////////////////////////////////////////////////////////////
bool CDistCalculator::HasStackOrOlderKnownHeldFrom(CBaseObject* pcObject)
{
	SHeapFromIterator	sIter;
	CBaseObject*		pcFrom;

	if (pcObject->HasStackFroms())
	{
		return true;
	}
	pcFrom = pcObject->StartHeapFromIteration(&sIter);
	while (pcFrom)
	{
		if (pcFrom->IsKnownHeldFromStack() && pcFrom->IsOlderThan(pcObject))
		{
			return true;
		}
		pcFrom = pcObject->IterateHeapFrom(&sIter);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether something that points to pcObject (or to what points to it...)
// is pointed to from the stack or attached to the root.  Then a stack
// pointer to pcObject going frees nothing, and the full distance to the
// stack walk (everything below it, then everything above that) isn't
// needed.  Breadth first, so the holder nearest is found first: building a
// component under a parent held on the stack finds it straight away.  It is
// a walk of its own: the objects it tests are stamped, so nothing needs
// clearing afterwards.  When pcObject has no stack froms, a from known
// held from the stack is held (if that relies on pcObject, pcObject is
// reached from the stack); while it has them, that may rely on them, so
// only the walk can say.
//
//////////////////////////////////////////////////////////////////////////
bool CDistCalculator::IsHeldFromStackOrRoot(CBaseObject* pcObject)
{
	CArrayBaseObjectPtr		apcFroms;
	CBaseObject*			pcFrom;
	size					uiFirst;
	bool					bHeld;
	uint32					uiTested;
	bool					bUseKnownHeld;

	uiTested = gcObjects.StartWalk() | WALK_STATE_TOUCHED;
	bUseKnownHeld = !pcObject->HasStackFroms();

	bHeld = false;
	apcFroms.Init();
	pcObject->SetWalkStamp(uiTested);
	pcObject->AddHeapFromContainers(&apcFroms);
	for (uiFirst = 0; uiFirst < apcFroms.NumElements(); uiFirst++)
	{
		pcFrom = apcFroms.GetPtr(uiFirst);
		if ((pcFrom->GetWalkStamp() == uiTested) || (pcFrom->GetFlags() & OBJECT_FLAGS_FREED))
		{
			continue;
		}
		if ((pcFrom->GetDistToRoot() >= ROOT_DIST_TO_ROOT) || pcFrom->HasStackFroms() || (bUseKnownHeld && pcFrom->IsKnownHeldFromStack()))
		{
			bHeld = true;
			break;
		}
		pcFrom->SetWalkStamp(uiTested);
		pcFrom->AddHeapFromContainers(&apcFroms);
	}
	apcFroms.Kill();
	return bHeld;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::InitialiseCompletelyDetached(void)
{
	size			i;
	size			uiNumDetached;
	CBaseObject*	pcBaseObject;

	uiNumDetached = NumDetachedFromRoot();
	for (i = 0; i < uiNumDetached; i++)
	{
		pcBaseObject = GetDetachedFromRoot(i);
		if (!IsReachedFromStack(pcBaseObject))
		{
			//Copy everything not reached from the stack into completely detached.
			//They'll be moved out as they are reached.
			AddCompletelyDetached(pcBaseObject);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether something pointing to pcObject (or to an object embedded in it)
// is reached from the stack.
//
//////////////////////////////////////////////////////////////////////////
bool CDistCalculator::HasFromReachedFromStack(CBaseObject* pcObject)
{
	SHeapFromIterator	sIter;
	CBaseObject*		pcFrom;

	pcFrom = pcObject->StartHeapFromIteration(&sIter);
	while (pcFrom)
	{
		if (IsReachedFromStack(pcFrom->GetEmbeddingContainer()))
		{
			return true;
		}
		pcFrom = pcObject->IterateHeapFrom(&sIter);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the completely detached objects pointed to from something reached
// from the stack, moving them to papcReached.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::MarkPointedToFromReached(CArrayBaseObjectPtr* papcReached)
{
	size			i;
	CBaseObject*	pcBaseObject;

	for (i = NumCompletelyDetached(); i > 0; i--)
	{
		pcBaseObject = GetCompletelyDetached(i - 1);
		if (HasFromReachedFromStack(pcBaseObject))
		{
			MarkReachedFromStack(pcBaseObject);
			RemoveCompletelyDetached(i - 1);
			papcReached->Add(pcBaseObject);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Finds the completely detached objects that can still be reached from
// the stack.  First those pointed to from something reached, then what
// they point to, breadth first.  (Going over the detached objects again
// and again until none changed took a pass per object along a chain.)
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::FindReachedFromStack(void)
{
	CArrayBaseObjectPtr			apcReached;
	CArrayEmbeddedObjectPtr*	papcTos;
	CBaseObject*				pcTo;
	size						uiFirst;
	size						i;
	size						uiNumTos;

	apcReached.Init();
	MarkPointedToFromReached(&apcReached);

	for (uiFirst = 0; uiFirst < apcReached.NumElements(); uiFirst++)
	{
		papcTos = StartPointerTos();
		apcReached.GetPtr(uiFirst)->GetPointerTos(papcTos);
		uiNumTos = papcTos->NumElements();
		for (i = 0; i < uiNumTos; i++)
		{
			pcTo = papcTos->GetPtr(i)->GetEmbeddingContainer();

			//Only objects this walk found detached from the root (the others are attached, or being freed).
			if (GetWalkState(pcTo) == WALK_STATE_DETACHED_WALKED)
			{
				MarkReachedFromStack(pcTo);
				apcReached.Add(pcTo);
			}
		}
	}
	apcReached.Kill();

	for (i = NumCompletelyDetached(); i > 0; i--)
	{
		if (IsReachedFromStack(GetCompletelyDetached(i - 1)))
		{
			RemoveCompletelyDetached(i - 1);
		}
	}
}

#include "BaseObject.h"
#include "DistToRootCalculator.h"
#include "DistToStackCalculator.h"


//////////////////////////////////////////////////////////////////////////
//
// A stack from of pcFromRemoved went.  Clears the distances to the root
// that can't be found any more (an object loaded without the root may
// have none to clear), then finds what can't be reached at all.
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::Calculate(CBaseObject* pcFromRemoved, CDistCalculatorParameters* pcParameters)
{
	CDistToRootCalculator	cDistToRootCalculator;

	cDistToRootCalculator.CollectAndClearInvalidDistToRoot(pcFromRemoved, pcParameters);
	cDistToRootCalculator.ResetClearedToUnattached(pcParameters);
	CalculateFromTouched(pcParameters);
	pcParameters->ClearTouchedFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::CalculateFromTouched(CDistCalculatorParameters* pcParameters)
{
	size	uiNumWithStackPointers;

	uiNumWithStackPointers = CollectDetachedFromRoot(pcParameters);

	if (uiNumWithStackPointers == 0)
	{
		//Kill them all!
		pcParameters->CopyRootDetachedToCompletelyDetached();
	}
	else if (uiNumWithStackPointers == pcParameters->NumDetachedFromRoot())
	{
		//Kill none of them.
	}
	else
	{
		InitialiseCompletelyDetached(pcParameters);
		MarkReachedFromStack(pcParameters);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Collects the touched objects, and the objects that point to them (and to
// them...), that are detached from the root.  Returns how many of them are
// pointed to from the stack.
//
//////////////////////////////////////////////////////////////////////////
size CDistToStackCalculator::CollectDetachedFromRoot(CDistCalculatorParameters* pcParameters)
{
	size					i;
	CBaseObject*			pcBaseObject;
	size					uiNumWithStackPointers;
	CArrayBaseObjectPtr		apcFroms;

	uiNumWithStackPointers = 0;
	apcFroms.Init();
	for (i = 0; i < pcParameters->NumTouched(); i++)
	{
		pcBaseObject = pcParameters->GetTouched(i);
		//I'm not sure if this is the right way to detect that object was declared on the stack.
		//And if on the stack it should not (ultimately) be attempted to be freed from Objects.
		if (pcBaseObject->IsAllocatedInObjects())
		{
			uiNumWithStackPointers += CollectDetachedFromRoot(pcBaseObject, pcParameters, &apcFroms);
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
size CDistToStackCalculator::CollectDetachedFromRoot(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters, CArrayBaseObjectPtr* papcFroms)
{
	size	uiNumWithStackPointers;

	uiNumWithStackPointers = 0;
	papcFroms->Add(pcObject);
	while (papcFroms->NumElements() > 0)
	{
		pcObject = papcFroms->Pop();
		if (CollectDetached(pcObject, pcParameters, papcFroms))
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
bool CDistToStackCalculator::CollectDetached(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters, CArrayBaseObjectPtr* papcFroms)
{
	size			uiFirst;
	size			uiLast;
	CBaseObject*	pcSwap;
	bool			bHasStackFroms;

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
	if (pcObject->GetFlags() & (OBJECT_FLAGS_DIST_FROM_WALKED | OBJECT_FLAGS_FREED))
	{
		return false;
	}

	pcParameters->AddDetachedFromRoot(pcObject);

	bHasStackFroms = pcObject->HasStackFroms();
	if (bHasStackFroms)
	{
		pcParameters->MarkReachedFromStack(pcObject);
	}

	if (!(pcObject->GetFlags() & OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED))
	{
		pcObject->SetFlag(OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED, true);
		pcParameters->AddTouched(pcObject);
	}

	pcObject->SetFlag(OBJECT_FLAGS_DIST_FROM_WALKED, true);

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
// Whether something that points to pcObject (or to what points to it...)
// is pointed to from the stack or attached to the root.  Then a stack
// pointer to pcObject going frees nothing, and the full distance to the
// stack walk (everything below it, then everything above that) isn't
// needed.  Breadth first, so the holder nearest is found first: building a
// component under a parent held on the stack finds it straight away.
//
//////////////////////////////////////////////////////////////////////////
bool CDistToStackCalculator::IsHeldFromStackOrRoot(CBaseObject* pcObject)
{
	CArrayBaseObjectPtr		apcFroms;
	CBaseObject*			pcFrom;
	size					uiFirst;
	size					i;
	bool					bHeld;

	bHeld = false;
	apcFroms.Init();
	pcObject->SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, true);
	apcFroms.Add(pcObject);
	pcObject->AddHeapFromContainers(&apcFroms);
	for (uiFirst = 1; uiFirst < apcFroms.NumElements(); uiFirst++)
	{
		pcFrom = apcFroms.GetPtr(uiFirst);
		if (pcFrom->GetFlags() & (OBJECT_FLAGS_TESTED_FOR_ROOT | OBJECT_FLAGS_FREED))
		{
			continue;
		}
		if ((pcFrom->GetDistToRoot() >= ROOT_DIST_TO_ROOT) || pcFrom->HasStackFroms())
		{
			bHeld = true;
			break;
		}
		pcFrom->SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, true);
		pcFrom->AddHeapFromContainers(&apcFroms);
	}

	//Everything walked (and only that) was flagged, and every flagged object is in the array.
	for (i = 0; i < apcFroms.NumElements(); i++)
	{
		apcFroms.GetPtr(i)->SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, false);
	}
	apcFroms.Kill();
	return bHeld;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::InitialiseCompletelyDetached(CDistCalculatorParameters* pcParameters)
{
	size			i;
	size			uiNumDetached;
	CBaseObject*	pcBaseObject;

	uiNumDetached = pcParameters->NumDetachedFromRoot();
	for (i = 0; i < uiNumDetached; i++)
	{
		pcBaseObject = pcParameters->GetDetachedFromRoot(i);
		if (!pcParameters->IsReachedFromStack(pcBaseObject))
		{
			//Copy everything not reached from the stack into completely detached.
			//They'll be moved out as they are reached.
			pcParameters->AddCompletelyDetached(pcBaseObject);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether something pointing to pcObject (or to an object embedded in it)
// is reached from the stack.  The froms are read where they are rather
// than copied: a hub has thousands.
//
//////////////////////////////////////////////////////////////////////////
bool CDistToStackCalculator::HasFromReachedFromStack(CBaseObject* pcObject, CDistCalculatorParameters* pcParameters)
{
	CEmbeddedObject*	pcHolder;
	size				uiHolder;
	size				uiNumHolders;
	size				i;
	size				uiNumFroms;

	uiNumHolders = pcObject->NumHeapFromHolders();
	for (uiHolder = 0; uiHolder < uiNumHolders; uiHolder++)
	{
		pcHolder = (uiHolder == 0) ? pcObject : pcObject->GetEmbeddedObject(uiHolder);
		uiNumFroms = pcHolder->CEmbeddedObject::NumHeapFroms();
		for (i = 0; i < uiNumFroms; i++)
		{
			if (pcParameters->IsReachedFromStack(pcHolder->CEmbeddedObject::GetHeapFrom(i)->GetEmbeddingContainer()))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the completely detached objects pointed to from something reached
// from the stack, moving them to papcReached.
//
//////////////////////////////////////////////////////////////////////////
void CDistToStackCalculator::MarkPointedToFromReached(CDistCalculatorParameters* pcParameters, CArrayBaseObjectPtr* papcReached)
{
	size			i;
	CBaseObject*	pcBaseObject;

	for (i = pcParameters->NumCompletelyDetached(); i > 0; i--)
	{
		pcBaseObject = pcParameters->GetCompletelyDetached(i - 1);
		if (HasFromReachedFromStack(pcBaseObject, pcParameters))
		{
			pcParameters->MarkReachedFromStack(pcBaseObject);
			pcParameters->RemoveCompletelyDetached(i - 1);
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
void CDistToStackCalculator::MarkReachedFromStack(CDistCalculatorParameters* pcParameters)
{
	CArrayBaseObjectPtr			apcReached;
	CArrayEmbeddedObjectPtr*	papcTos;
	CBaseObject*				pcTo;
	size						uiFirst;
	size						i;
	size						uiNumTos;

	apcReached.Init();
	MarkPointedToFromReached(pcParameters, &apcReached);

	for (uiFirst = 0; uiFirst < apcReached.NumElements(); uiFirst++)
	{
		papcTos = pcParameters->StartPointerTos();
		apcReached.GetPtr(uiFirst)->GetPointerTos(papcTos);
		uiNumTos = papcTos->NumElements();
		for (i = 0; i < uiNumTos; i++)
		{
			pcTo = papcTos->GetPtr(i)->GetEmbeddingContainer();

			//Only objects this walk found detached from the root (the others are attached, or being freed).
			if ((pcTo->GetFlags() & OBJECT_FLAGS_DIST_FROM_WALKED) && !pcParameters->IsReachedFromStack(pcTo))
			{
				pcParameters->MarkReachedFromStack(pcTo);
				apcReached.Add(pcTo);
			}
		}
	}
	apcReached.Kill();

	for (i = pcParameters->NumCompletelyDetached(); i > 0; i--)
	{
		if (pcParameters->IsReachedFromStack(pcParameters->GetCompletelyDetached(i - 1)))
		{
			pcParameters->RemoveCompletelyDetached(i - 1);
		}
	}
}

#include "BaseLib/PointerFunctions.h"
#include "BaseObject.h"
#include "Objects.h"
#include "DistCalculator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::Init(void)
{
	muiWalk = gcObjects.StartWalk();
	mapcTouched.Init();
	mapcTos.Init();
	mapcDetachedFromRoot.Init();
	mapcCompletelyDetached.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::Kill(void)
{
	mapcCompletelyDetached.Kill();
	mapcDetachedFromRoot.Kill();
	mapcTos.Kill();
	mapcTouched.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A from of pcFromChanged was removed (a heap from if bHeapFromChanged,
// otherwise a stack from).  Returns the objects that can no longer be
// reached and must be freed, in the order the walk found them (not by
// address: the order their Free()s are called in is then the same on
// every platform and run).
//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CDistCalculator::Calculate(CBaseObject* pcFromChanged, bool bHeapFromChanged)
{
	if (bHeapFromChanged)
	{
		CalculateDistToRoot(pcFromChanged);
		CalculateFromTouched();
	}
	else if (!pcFromChanged->HasStackFroms() && !RecalculateKnownHeldFromStack(pcFromChanged) && !IsHeldFromStackOrRoot(pcFromChanged))
	{
		CalculateDistToStack(pcFromChanged);
	}
	return &mapcCompletelyDetached;
}


//////////////////////////////////////////////////////////////////////////
//
// An empty array for one object's pointer tos.  The same one each time, so
// walking many objects doesn't allocate for each.
//
//////////////////////////////////////////////////////////////////////////
CArrayEmbeddedObjectPtr* CDistCalculator::StartPointerTos(void)
{
	mapcTos.FakeSetUsedElements(0);
	return &mapcTos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::AddDetachedFromRoot(CBaseObject* pcObject)
{
	mapcDetachedFromRoot.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculator::NumDetachedFromRoot(void)
{
	return mapcDetachedFromRoot.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculator::GetDetachedFromRoot(size iIndex)
{
	return mapcDetachedFromRoot.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::AddCompletelyDetached(CBaseObject* pcObject)
{
	mapcCompletelyDetached.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculator::NumCompletelyDetached(void)
{
	return mapcCompletelyDetached.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculator::GetCompletelyDetached(size iIndex)
{
	return mapcCompletelyDetached.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::RemoveCompletelyDetached(size iIndex)
{
	mapcCompletelyDetached.RemoveAt(iIndex, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::CopyRootDetachedToCompletelyDetached(void)
{
	//Poor mans copy for now.  Should resize and memcpy rather.
	size			i;
	size			iNum;
	CBaseObject*	pcBaseObject;

	iNum = mapcDetachedFromRoot.NumElements();
	for (i = 0; i < iNum; i++)
	{
		pcBaseObject = *mapcDetachedFromRoot.Get(i);
		mapcCompletelyDetached.Add(pcBaseObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CDistCalculator::GetCompletelyDetachedArray(void)
{
	return &mapcCompletelyDetached;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::AddTouched(CBaseObject* pcObject)
{
	mapcTouched.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculator::NumTouched(void)
{
	return mapcTouched.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculator::GetTouched(size iIndex)
{
	return mapcTouched.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::PrintArray(CChars* psz, CArrayBaseObjectPtrE32* pcArray, bool bCommaSeparate)
{
	size			i;
	size			iNum;
	CBaseObject*	pcBaseObject;

	iNum = pcArray->NumElements();
	for (i = 0; i < iNum; i++)
	{
		if (!bCommaSeparate)
		{
			psz->Append("  ");
		}

		pcBaseObject = *pcArray->Get(i);
		pcBaseObject->PrintObject(psz, false);
		
		if (bCommaSeparate)
		{
			if (i != iNum - 1)
			{
				psz->Append(", ");
			}
		}
		else
		{
			psz->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::PrintArray(CChars* psz, CArrayBaseObjectPtr* pcArray, bool bCommaSeparate)
{
	size			i;
	size			iNum;
	CBaseObject*	pcBaseObject;

	iNum = pcArray->NumElements();
	for (i = 0; i < iNum; i++)
	{
		if (!bCommaSeparate)
		{
			psz->Append("  ");
		}

		pcBaseObject = *pcArray->Get(i);
		pcBaseObject->PrintObject(psz, false);

		if (bCommaSeparate)
		{
			if (i != iNum - 1)
			{
				psz->Append(", ");
			}
		}
		else
		{
			psz->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What this calculation found the object to be (a WALK_STATE_...), or 0
// if it hasn't reached it.
//
//////////////////////////////////////////////////////////////////////////
uint32 CDistCalculator::GetWalkState(CBaseObject* pcObject)
{
	uint32	uiStamp;

	uiStamp = pcObject->GetWalkStamp();
	if ((uiStamp & ~WALK_STAMP_STATE_MASK) != muiWalk)
	{
		return 0;
	}
	return uiStamp & WALK_STAMP_STATE_MASK;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::SetWalkState(CBaseObject* pcObject, uint32 uiState)
{
	pcObject->SetWalkStamp(muiWalk | uiState);
}


//////////////////////////////////////////////////////////////////////////
//
// The object is detached from the root but can be reached from the stack.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::MarkReachedFromStack(CBaseObject* pcObject)
{
	SetWalkState(pcObject, WALK_STATE_REACHED_FROM_STACK);
}


//////////////////////////////////////////////////////////////////////////
//
// (Heap froms are only ever objects allocated in Objects: pointers and
// collections in an object on the stack are stack froms.)
//
//////////////////////////////////////////////////////////////////////////
bool CDistCalculator::IsReachedFromStack(CBaseObject* pcObject)
{
	return pcObject->GetWalkStamp() == (muiWalk | WALK_STATE_REACHED_FROM_STACK);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::Print(CChars* psz, bool bCommaSeparate)
{
	psz->Append("------------- DistCalculator ---------------");
	psz->AppendNewLine();

	if (bCommaSeparate)
	{
		psz->Append("      ");
	}
	psz->Append("Touched [");
	psz->Append(mapcTouched.NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintArray(psz, &mapcTouched, bCommaSeparate);

	psz->AppendNewLine();

	if (bCommaSeparate)
	{
		psz->Append("     ");
	}
	psz->Append("Detached [");
	psz->Append(mapcDetachedFromRoot.NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintArray(psz, &mapcDetachedFromRoot, bCommaSeparate);
	psz->AppendNewLine();

	if (bCommaSeparate)
	{
		psz->Append("  ");
	}
	psz->Append("C. Detached [");
	psz->Append(mapcCompletelyDetached.NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintArray(psz, &mapcCompletelyDetached, bCommaSeparate);
	psz->AppendNewLine();

	psz->Append("--------------------------------------------");
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::Dump(void)
{
	CChars sz;

	sz.Init();
	Print(&sz, false);
	sz.DumpKill();
}

#include "BaseObject.h"
#include "Objects.h"
#include "DistCalculatorParameters.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::Init(void)
{
	muiWalk = gcObjects.StartWalk();
	mapcTos.Init();
	mapcDetachedFromRoot.Init();
	mapcCompletelyDetached.Init();
	mapcTouched.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::Kill(void)
{
	mapcTouched.Kill();
	mapcCompletelyDetached.Kill();
	mapcDetachedFromRoot.Kill();
	mapcTos.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An empty array for one object's pointer tos.  The same one each time, so
// walking many objects doesn't allocate for each.
//
//////////////////////////////////////////////////////////////////////////
CArrayEmbeddedObjectPtr* CDistCalculatorParameters::StartPointerTos(void)
{
	mapcTos.FakeSetUsedElements(0);
	return &mapcTos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddDetachedFromRoot(CBaseObject* pcObject)
{
	mapcDetachedFromRoot.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculatorParameters::NumDetachedFromRoot(void)
{
	return mapcDetachedFromRoot.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetDetachedFromRoot(size iIndex)
{
	return mapcDetachedFromRoot.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddCompletelyDetached(CBaseObject* pcObject)
{
	mapcCompletelyDetached.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculatorParameters::NumCompletelyDetached(void)
{
	return mapcCompletelyDetached.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetCompletelyDetached(size iIndex)
{
	return mapcCompletelyDetached.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::RemoveCompletelyDetached(size iIndex)
{
	mapcCompletelyDetached.RemoveAt(iIndex, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::CopyRootDetachedToCompletelyDetached(void)
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
CArrayBaseObjectPtr* CDistCalculatorParameters::GetCompletelyDetachedArray(void)
{
	return &mapcCompletelyDetached;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::AddTouched(CBaseObject* pcObject)
{
	mapcTouched.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDistCalculatorParameters::NumTouched(void)
{
	return mapcTouched.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CDistCalculatorParameters::GetTouched(size iIndex)
{
	return mapcTouched.GetPtr(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::PrintArray(CChars* psz, CArrayBaseObjectPtrE32* pcArray, bool bCommaSeparate)
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
void CDistCalculatorParameters::PrintArray(CChars* psz, CArrayBaseObjectPtr* pcArray, bool bCommaSeparate)
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
uint32 CDistCalculatorParameters::GetWalkState(CBaseObject* pcObject)
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
void CDistCalculatorParameters::SetWalkState(CBaseObject* pcObject, uint32 uiState)
{
	pcObject->SetWalkStamp(muiWalk | uiState);
}


//////////////////////////////////////////////////////////////////////////
//
// The object is detached from the root but can be reached from the stack.
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::MarkReachedFromStack(CBaseObject* pcObject)
{
	SetWalkState(pcObject, WALK_STATE_REACHED_FROM_STACK);
}


//////////////////////////////////////////////////////////////////////////
//
// (Heap froms are only ever objects allocated in Objects: pointers and
// collections in an object on the stack are stack froms.)
//
//////////////////////////////////////////////////////////////////////////
bool CDistCalculatorParameters::IsReachedFromStack(CBaseObject* pcObject)
{
	return pcObject->GetWalkStamp() == (muiWalk | WALK_STATE_REACHED_FROM_STACK);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculatorParameters::Print(CChars* psz, bool bCommaSeparate)
{
	psz->Append("--------- DistCalculatorParameters ---------");
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
void CDistCalculatorParameters::Dump(void)
{
	CChars sz;

	sz.Init();
	Print(&sz, false);
	sz.DumpKill();
}

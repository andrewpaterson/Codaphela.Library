/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/PointerFunctions.h"
#include "BaseObject.h"
#include "Objects.h"
#include "DistCalculator.h"


#define DIST_CALCULATOR_PRINT_TITLE_WIDTH	13	//Print's section titles are right aligned to this, on their objects' line.


//////////////////////////////////////////////////////////////////////////
//
// Prints each object in pcArray (any of the object pointer arrays):
// separated by commas, or each on its own indented line.
//
//////////////////////////////////////////////////////////////////////////
template<class TArray>
static void PrintObjects(CChars* psz, TArray* pcArray, bool bCommaSeparate)
{
	size	i;
	size	uiNum;

	uiNum = pcArray->NumElements();
	for (i = 0; i < uiNum; i++)
	{
		if (!bCommaSeparate)
		{
			psz->Append("  ");
		}

		(*pcArray->Get(i))->PrintObject(psz, false);

		if (!bCommaSeparate)
		{
			psz->AppendNewLine();
		}
		else if (i + 1 != uiNum)
		{
			psz->Append(", ");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One of Print's sections: its title and count, then its objects (on the
// same line, the titles right aligned, when bCommaSeparate).
//
//////////////////////////////////////////////////////////////////////////
template<class TArray>
static void PrintSection(CChars* psz, const char* szTitle, TArray* pcArray, bool bCommaSeparate)
{
	if (bCommaSeparate)
	{
		psz->RightAlign(szTitle, ' ', DIST_CALCULATOR_PRINT_TITLE_WIDTH);
	}
	else
	{
		psz->Append(szTitle);
	}
	psz->Append(" [");
	psz->Append(pcArray->NumElements());
	psz->Append("]:  ");
	if (!bCommaSeparate)
	{
		psz->AppendNewLine();
	}
	PrintObjects(psz, pcArray, bCommaSeparate);
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDistCalculator::Init(CObjects* pcObjects)
{
	mpcObjects = pcObjects;
	muiWalk = mpcObjects->StartWalk();
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
	PrintSection(psz, "Touched", &mapcTouched, bCommaSeparate);
	PrintSection(psz, "Detached", &mapcDetachedFromRoot, bCommaSeparate);
	PrintSection(psz, "C. Detached", &mapcCompletelyDetached, bCommaSeparate);
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

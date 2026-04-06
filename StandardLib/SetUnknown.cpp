/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "SortPointersHelper.h"
#include "SetUnknown.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetUnknown::Init(bool bSortPointers)
{

	CArrayCommonUnknown::Init(false, true, false, true, bSortPointers, CalculateDataCompareForSortPoiners(bSortPointers));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetUnknown::Init(size iChunkSize, bool bSortPointers)
{
	CArrayCommonUnknown::Init(false, true, false, true, bSortPointers, CalculateDataCompareForSortPoiners(bSortPointers), iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetUnknown::Init(bool bKillElements, bool bUnique, bool bIgnoreNull, bool bPreserveOrder, size iChunkSize, bool bSortPointers)
{
	CArrayCommonUnknown::Init(false, bKillElements, bUnique, bIgnoreNull, bPreserveOrder, CalculateDataCompareForSortPoiners(bSortPointers), iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetUnknown::Kill(void)
{
	CArrayCommonUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetUnknown::Add(CUnknown* pcUnknown)
{
	if (!mfCompare)
	{
		CArrayCommonUnknown::Add(pcUnknown);
	}
	else
	{
		CArrayCommonUnknown::InsertIntoSorted(mfCompare, pcUnknown, muiFlags & ARRAY_COMMOM_UNIQUE_ONLY);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSetUnknown::Remove(CUnknown* pcUnknown)
{
	return CArrayCommonUnknown::Remove(pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetUnknown::RemoveDuringIteration(SSetIterator* psIter)
{
	CArrayCommonUnknown::RemoveDuringIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetUnknown::StartIteration(SSetIterator* psIter, CUnknown** ppcUnknown)
{
	return CArrayCommonUnknown::StartIteration(psIter, ppcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSetUnknown::Iterate(SSetIterator* psIter, CUnknown** ppcUnknown)
{
	return CArrayCommonUnknown::Iterate(psIter, ppcUnknown);
}


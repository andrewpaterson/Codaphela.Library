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
#include "SetUnknown.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetUnknown::Init(bool bSortPointers)
{
	CArrayCommonUnknown::Init(Flags(false, true, false, true, false, bSortPointers));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetUnknown::Init(size iChunkSize, bool bSortPointers)
{
	CArrayCommonUnknown::Init(Flags(false, true, false, true, false, bSortPointers), iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetUnknown::Init(bool bKillElements, bool bUnique, bool bIgnoreNull, bool bPreserveOrder, size iChunkSize, bool bSortPointers)
{
	CArrayCommonUnknown::Init(Flags(false, bKillElements, bUnique, bIgnoreNull, bPreserveOrder, bSortPointers), iChunkSize);
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
// False if pcUnknown didn't go in: a NULL where NULLs are ignored, or an
// element a unique set has already.
//
//////////////////////////////////////////////////////////////////////////
bool CSetUnknown::Add(CUnknown* pcUnknown)
{
	if (!IsMustSort())
	{
		return CArrayCommonUnknown::Add(pcUnknown);
	}

	if ((muiFlags & ARRAY_COMMON_UNIQUE_ONLY) && Contains(pcUnknown))
	{
		return false;
	}
	return CArrayCommonUnknown::InsertIntoSorted(pcUnknown) != ARRAY_ELEMENT_NOT_FOUND;
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
bool CSetUnknown::RemoveDuringIteration(SSetIterator* psIter)
{
	return CArrayCommonUnknown::RemoveDuringIteration(psIter);
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


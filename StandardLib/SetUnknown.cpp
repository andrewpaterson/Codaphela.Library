/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "SetUnknown.h"
#include "BaseLib/PointerFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSetUnknown::Init(int iChunkSize)
{
	CArrayCommonUnknown::Init(FALSE, TRUE, FALSE, TRUE, FALSE, iChunkSize);
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
	CArrayCommonUnknown::Add(pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CSetUnknown::Remove(CUnknown* pcUnknown)
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
CUnknown* CSetUnknown::StartIteration(SSetIterator* psIter)
{
	return CArrayCommonUnknown::StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CSetUnknown::Iterate(SSetIterator* psIter)
{
	return CArrayCommonUnknown::Iterate(psIter);
}


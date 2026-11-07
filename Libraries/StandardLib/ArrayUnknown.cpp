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
#include "ArrayUnknown.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayUnknown::Init(void)
{
	CArrayCommonUnknown::Init(ARRAY_COMMON_KILL_ELEMENT | ARRAY_COMMON_PRESERVE_ORDER);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayUnknown::Kill(void)
{
	CArrayCommonUnknown::Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayUnknown::Add(CUnknown* pcUnknown)
{
	return CArrayCommonUnknown::Add(pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayUnknown::Insert(size iIndex, CUnknown* pcUnknown)
{
	return CArrayCommonUnknown::Insert(iIndex, pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CArrayUnknown::Get(size iIndex)
{
	return CArrayCommonUnknown::UnsafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayUnknown::Set(size iIndex, CUnknown* pcUnknown)
{
	//Kills what was at iIndex: the array kills its elements.
	return CArrayCommonUnknown::Set(iIndex, pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayUnknown::Remove(size iIndex)
{
	return CArrayCommonUnknown::Remove(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayUnknown::Remove(CUnknown* pcUnknown)
{
	return CArrayCommonUnknown::Remove(pcUnknown);
}


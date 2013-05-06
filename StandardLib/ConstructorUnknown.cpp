/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "ConstructorUnknown.h"
#include "Unknowns.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstructorUnknown::Init(void)
{
	mcConstructorObjects.Init(FALSE, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstructorUnknown::Kill(void)
{
	mcConstructorObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstructorUnknown::AddUnknown(CUnknown* pcUnknown)
{
	pcUnknown = gcUnknowns.AddExisting(pcUnknown);
	mcConstructorObjects.Put(pcUnknown->ClassName(), pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConstructorUnknown::Construct(CUnknown* pcDest, char* szName)
{
	CUnknown*	pcConstructor;

	pcConstructor = mcConstructorObjects.Get(szName);
	if (!pcConstructor)
	{
		return FALSE;
	}

	memcpy(pcDest, pcConstructor, pcConstructor->ClassSize());
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CConstructorUnknown::GetUnknown(char* szName)
{
	CUnknown*	pcConstructor;

	pcConstructor = mcConstructorObjects.Get(szName);
	return pcConstructor;
}


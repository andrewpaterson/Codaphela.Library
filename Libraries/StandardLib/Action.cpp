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
#include "Action.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAction::Init(const char* szName)
{
	mcCalls.Init();
	mszName.Init(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAction::Kill(void)
{
	mszName.Kill();
	mcCalls.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Removes every function added for pvObject.  Returns false if there
// were none.
//
//////////////////////////////////////////////////////////////////////////
bool CAction::Remove(void* pvObject)
{
	return mcCalls.Remove(pvObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAction::NumCalls(void)
{
	return mcCalls.NumCalls();
}


//////////////////////////////////////////////////////////////////////////
//
// Calls each function in the order they were added (see
// CMemberCalls::Call).
//
//////////////////////////////////////////////////////////////////////////
void CAction::Call(void* pvContext)
{
	mcCalls.Call(this, &pvContext);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAction::Is(const char* szName)
{
	return mszName.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAction::GetName(void)
{
	return mszName.Text();
}


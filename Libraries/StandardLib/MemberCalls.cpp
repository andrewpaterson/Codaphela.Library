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
#include "MemberCalls.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemberCalls::Init(void)
{
	masCalls.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemberCalls::Kill(void)
{
	masCalls.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a call, called after those already added.  Init it with the
// object and its function.
//
//////////////////////////////////////////////////////////////////////////
SMemberCall* CMemberCalls::Add(void)
{
	return masCalls.Add();
}


//////////////////////////////////////////////////////////////////////////
//
// Removes every call on pvObject.  Returns false if there were none.
//
//////////////////////////////////////////////////////////////////////////
bool CMemberCalls::Remove(void* pvObject)
{
	size	i;
	bool	bRemoved;

	bRemoved = false;
	for (i = masCalls.NumElements(); i > 0; i--)
	{
		if (masCalls.Get(i - 1)->IsOn(pvObject))
		{
			masCalls.RemoveAt(i - 1);
			bRemoved = true;
		}
	}
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemberCalls::NumCalls(void)
{
	return masCalls.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// Calls each function in the order they were added, giving it what it
// takes of pvArgument and the value pvValue points to.  A function can
// add or remove calls; one removed before its turn isn't called, and one
// added isn't called until the next time.
//
//////////////////////////////////////////////////////////////////////////
void CMemberCalls::Call(void* pvArgument, void* pvValue)
{
	CArrayTemplate<SMemberCall>	asCalls;
	SMemberCall*				psCall;
	size						i;

	asCalls.Init();
	asCalls.Copy(&masCalls);
	for (i = 0; i < asCalls.NumElements(); i++)
	{
		psCall = asCalls.Get(i);
		if (Contains(psCall))
		{
			psCall->Call(pvArgument, pvValue);
		}
	}
	asCalls.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemberCalls::Contains(SMemberCall* psCall)
{
	size	i;

	for (i = 0; i < masCalls.NumElements(); i++)
	{
		if (masCalls.Get(i)->Equals(psCall))
		{
			return true;
		}
	}
	return false;
}


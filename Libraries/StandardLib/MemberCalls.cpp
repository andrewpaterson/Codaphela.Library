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
	muiCallDepth = 0;
	muiNumCleared = 0;
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
// While the calls are running they are cleared instead, so the calls
// still to come stay where Call will look for them.
//
//////////////////////////////////////////////////////////////////////////
bool CMemberCalls::Remove(void* pvObject)
{
	size			i;
	bool			bRemoved;
	SMemberCall*	psCall;

	bRemoved = false;
	for (i = masCalls.NumElements(); i > 0; i--)
	{
		psCall = masCalls.Get(i - 1);
		if (psCall->IsSet() && psCall->IsOn(pvObject))
		{
			if (muiCallDepth == 0)
			{
				masCalls.RemoveAt(i - 1);
			}
			else
			{
				psCall->Clear();
				muiNumCleared++;
			}
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
	return masCalls.NumElements() - muiNumCleared;
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
	SMemberCall*	psCall;
	SMemberCall		sCall;
	size			i;
	size			uiNumCalls;

	//Calls added from here on are after these.
	uiNumCalls = masCalls.NumElements();
	muiCallDepth++;
	for (i = 0; i < uiNumCalls; i++)
	{
		psCall = masCalls.Get(i);
		if (psCall->IsSet())
		{
			//Called from a copy: adding a call can move the array.
			sCall = *psCall;
			sCall.Call(pvArgument, pvValue);
		}
	}
	muiCallDepth--;

	if ((muiCallDepth == 0) && (muiNumCleared != 0))
	{
		RemoveCleared();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Takes out the calls removed while they were running, keeping the order
// of the rest.
//
//////////////////////////////////////////////////////////////////////////
void CMemberCalls::RemoveCleared(void)
{
	size	uiRead;
	size	uiWrite;
	size	uiNumCalls;

	uiNumCalls = masCalls.NumElements();
	uiWrite = 0;
	for (uiRead = 0; uiRead < uiNumCalls; uiRead++)
	{
		if (masCalls.Get(uiRead)->IsSet())
		{
			if (uiWrite != uiRead)
			{
				*masCalls.Get(uiWrite) = *masCalls.Get(uiRead);
			}
			uiWrite++;
		}
	}
	masCalls.Resize(uiWrite);
	muiNumCleared = 0;
}


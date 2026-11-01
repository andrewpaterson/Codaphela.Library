/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "BaseLib/PointerFunctions.h"
#include "LongTime.h"
#include "SimultaneousEvents.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSimultaneousEvents::Init(int64 iTime)
{
	miTime = iTime;
	mapcEvents.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The events aren't killed, see KillEvents.
//
//////////////////////////////////////////////////////////////////////////
void CSimultaneousEvents::Kill(void)
{
	mapcEvents.Kill();
	miTime = -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Kills and frees every event, the timeline owns them.
//
//////////////////////////////////////////////////////////////////////////
void CSimultaneousEvents::KillEvents(void)
{
	size		i;
	CEvent*		pcEvent;

	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		pcEvent = mapcEvents.GetPtr(i);
		SafeKill(pcEvent);
	}
	mapcEvents.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Fails if the event isn't at this time or has already been added.
//
//////////////////////////////////////////////////////////////////////////
bool CSimultaneousEvents::Add(CEvent* pcEvent)
{
	CChars	sz;

	if (pcEvent->GetTime() != miTime)
	{
		sz.Init(" Simultaneous time [");
		AppendNanoseconds(&sz, miTime);
		sz.Append("] is not equal to event time [");
		AppendNanoseconds(&sz, pcEvent->GetTime());
		sz.Append("].");
		gcLogger.Error2(__METHOD__, sz.Text(), NULL);
		sz.Kill();
		return false;
	}

	if (Contains(pcEvent))
	{
		sz.Init(" Event [");
		pcEvent->PrintShort(&sz);
		sz.Append("] has already been added.");
		gcLogger.Error2(__METHOD__, sz.Text(), NULL);
		sz.Kill();
		return false;
	}

	mapcEvents.Add(pcEvent);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The event isn't killed.  Returns false if it wasn't one of these.
//
//////////////////////////////////////////////////////////////////////////
bool CSimultaneousEvents::Remove(CEvent* pcEvent)
{
	size	i;

	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		if (mapcEvents.GetPtr(i) == pcEvent)
		{
			mapcEvents.RemoveAt(i, true);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSimultaneousEvents::Contains(CEvent* pcEvent)
{
	size	i;

	for (i = 0; i < mapcEvents.NumElements(); i++)
	{
		if (mapcEvents.GetPtr(i) == pcEvent)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEvent* CSimultaneousEvents::GetFirst(void)
{
	if (mapcEvents.NumElements() > 0)
	{
		return mapcEvents.GetPtr(0);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the time, the tree's key, is.  Worked out from an object as the
// event array makes the class one that offsetof isn't defined for.
//
//////////////////////////////////////////////////////////////////////////
size CSimultaneousEvents::GetTimeOffset(void)
{
	CSimultaneousEvents	cEvents;

	return (size)((uint8*)&cEvents.miTime - (uint8*)&cEvents);
}


//////////////////////////////////////////////////////////////////////////
//
// Compares two times, the keys of the timeline's red-black tree.
//
//////////////////////////////////////////////////////////////////////////
int CompareSimultaneousEventsTime(const void* pvLeft, const void* pvRight)
{
	int64	iLeft;
	int64	iRight;

	iLeft = *((int64*)pvLeft);
	iRight = *((int64*)pvRight);
	if (iLeft < iRight)
	{
		return -1;
	}
	else if (iLeft > iRight)
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSimultaneousEvents::NumEvents(void) { return mapcEvents.NumElements(); }
CEvent* CSimultaneousEvents::GetEvent(size uiIndex) { return mapcEvents.GetPtr(uiIndex); }
int64 CSimultaneousEvents::GetTime(void) { return miTime; }


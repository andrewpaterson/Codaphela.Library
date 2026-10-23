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
#include "Timeline.h"
#include "Event.h"


//The id the next event is given.
int64	giNextEventID = 1;


//////////////////////////////////////////////////////////////////////////
//
// The event is given the next id.
//
//////////////////////////////////////////////////////////////////////////
void CEvent::Init(int64 iTime)
{
	miTime = iTime;
	miID = giNextEventID;
	giNextEventID++;
	mbRemoved = false;
	mbExecuted = false;
}


//////////////////////////////////////////////////////////////////////////
//
// For an event that already has an id, as when it is loaded.  Later
// events are given ids after it.
//
//////////////////////////////////////////////////////////////////////////
void CEvent::Init(int64 iTime, int64 iID)
{
	miTime = iTime;
	miID = iID;
	if (iID >= giNextEventID)
	{
		giNextEventID = iID + 1;
	}
	mbRemoved = false;
	mbExecuted = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEvent::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Subclasses do their work then call this.  An executed event is removed
// from its owner too.
//
//////////////////////////////////////////////////////////////////////////
void CEvent::Execute(CTimeline* pcTimeline)
{
	mbExecuted = true;
	RemoveFromOwner();
}


//////////////////////////////////////////////////////////////////////////
//
// Subclasses with an owner that keeps a pointer to the event take it from
// the owner then call this.
//
//////////////////////////////////////////////////////////////////////////
void CEvent::RemoveFromOwner(void)
{
	mbRemoved = true;
}


//////////////////////////////////////////////////////////////////////////
//
// Like "Tick: Time [1024]", the class name without its C and Event.
//
//////////////////////////////////////////////////////////////////////////
void CEvent::PrintShort(CChars* psz)
{
	CChars	szName;

	szName.Init(ClassName());
	if (szName.StartsWith("C"))
	{
		szName.RemoveFromStart(1);
	}
	szName.Replace("Event", "");
	psz->Append(szName);
	psz->Append(": Time [");
	psz->Append(miTime);
	psz->Append("]");
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEvent::ResetNextID(void)
{
	giNextEventID = 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEvent::IsRemoved(void) { return mbRemoved; }
bool CEvent::IsExecuted(void) { return mbExecuted; }
int64 CEvent::GetTime(void) { return miTime; }
int64 CEvent::GetID(void) { return miID; }


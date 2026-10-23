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
#ifndef __SIMULTANEOUS_EVENTS_H__
#define __SIMULTANEOUS_EVENTS_H__
#include "BaseLib/RedBlackTree.h"
#include "Event.h"


//The events at one time on a CTimeline, like Logicim's SimultaneousEvents.  They are kept in the order they were added,
//which is the order they execute in.  An event can only be added once.
//
//The timeline keeps them in a red-black tree keyed by their time, see GetTimeOffset and CompareSimultaneousEventsTime.
//The tree copies them into its nodes, so they are initialised before they are added and killed before they are
//removed.
class CSimultaneousEvents
{
protected:
	int64			miTime;
	CArrayEventPtr	mapcEvents;

public:
	void	Init(int64 iTime);
	void	Kill(void);

	bool	Add(CEvent* pcEvent);
	bool	Remove(CEvent* pcEvent);
	bool	Contains(CEvent* pcEvent);
	size	NumEvents(void);
	CEvent*	GetEvent(size uiIndex);
	CEvent*	GetFirst(void);
	int64	GetTime(void);
	void	KillEvents(void);

	static size	GetTimeOffset(void);
};


int CompareSimultaneousEventsTime(const void* pvLeft, const void* pvRight);


typedef CRedBlackTreeTemplate<CSimultaneousEvents>	CSimultaneousEventsTree;


#endif // __SIMULTANEOUS_EVENTS_H__


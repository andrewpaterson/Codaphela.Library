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
#ifndef __TIMELINE_H__
#define __TIMELINE_H__
#include "SimultaneousEvents.h"


//What a timeline saves and loads, like Logicim's TimelineData.  The events are saved by their owners.
struct STimelineData
{
	int64	iTime;
	int64	iPreviousEventTime;
	int64	iEventTime;
};


//Runs events in time order, like Logicim's Timeline.  The events at each time are a CSimultaneousEvents, kept in a
//red-black tree keyed by their time, as Logicim keeps them.  Running moves the time on to the earliest events and executes them in the
//order they were added; an executing event can add events, but only at a later time.
//
//The timeline owns the events added to it, see CEvent.  An executed or removed event is killed and freed.  An event
//removed while the events at its time are executing, by one of them, isn't executed and is freed once they have all
//run.
//
//Logicim throws a SimulatorException when an event is in the past; here an error is logged and false returned.
class CTimeline
{
protected:
	int64							miTime;
	int64							miPreviousEventTime;
	int64							miEventTime;
	CSimultaneousEventsTree			mcEvents;  //Keyed by time, the earliest first.
	CSimultaneousEvents*			mpcRunning;  //The events executing now, NULL between runs.
	void*							mpvContext;  //Logicim's Simulation.  Events reach it through the timeline.

public:
	void					Init(void* pvContext = NULL);
	void					Kill(void);

	bool					AddFutureEvent(CEvent* pcEvent);
	bool					AddEvent(CEvent* pcEvent);

	void					Run(void);
	bool					RunToTime(int64 iTimeForward);
	bool					RunSimultaneous(void);

	bool					Remove(CEvent* pcEvent);
	bool					RemoveAll(CArrayEventPtr* papcEvents);

	int64					GetTime(void);
	int64					GetPreviousEventTime(void);
	int64					GetEventTime(void);
	size					NumSimultaneousEvents(void);
	CSimultaneousEvents*	GetFirstSimultaneousEvents(void);
	CSimultaneousEvents*	GetNextSimultaneousEvents(CSimultaneousEvents* pcEvents);
	CSimultaneousEvents*	FindSimultaneousEvents(int64 iTime);
	bool					IsValid(void);
	void*					GetContext(void);

	void					Save(STimelineData* psData);
	void					Load(STimelineData* psData);

protected:
	void					ExecuteEvents(CSimultaneousEvents* pcEvents);
	void					RemoveSimultaneousEvents(CSimultaneousEvents* pcEvents);
	void					LogEventTimeError(const char* szMethod, const char* szMessage, int64 iEventTime);
};


#endif // __TIMELINE_H__


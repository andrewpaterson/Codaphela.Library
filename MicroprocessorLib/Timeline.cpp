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
#include "Timeline.h"


//////////////////////////////////////////////////////////////////////////
//
// pvContext is given to the events through GetContext, the simulation the
// timeline is in say.
//
//////////////////////////////////////////////////////////////////////////
void CTimeline::Init(void* pvContext)
{
	miTime = 0;
	miPreviousEventTime = 0;
	miEventTime = 0;
	mcEvents.Init(CSimultaneousEvents::GetTimeOffset(), CompareSimultaneousEventsTime);
	mpcRunning = NULL;
	mpvContext = pvContext;
}


//////////////////////////////////////////////////////////////////////////
//
// The events still waiting are killed and freed without being removed
// from their owners.
//
//////////////////////////////////////////////////////////////////////////
void CTimeline::Kill(void)
{
	SRedBlackTreeIterator	sIter;
	CSimultaneousEvents*	pcEvents;

	pcEvents = mcEvents.StartIteration(&sIter);
	while (pcEvents)
	{
		pcEvents->KillEvents();
		pcEvents->Kill();
		pcEvents = mcEvents.Iterate(&sIter);
	}
	mcEvents.Kill();
	mpcRunning = NULL;
	mpvContext = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Logs an error if the event isn't in the future.  The caller still owns
// an event that isn't added; an event already added fails too.
//
//////////////////////////////////////////////////////////////////////////
bool CTimeline::AddFutureEvent(CEvent* pcEvent)
{
	CChars	sz;

	if (pcEvent->GetTime() <= miTime)
	{
		sz.Init(" Cannot add event in the past.  Event time [");
		AppendNanoseconds(&sz, pcEvent->GetTime());
		sz.Append("] must be after current time [");
		AppendNanoseconds(&sz, miTime);
		sz.Append("].");
		gcLogger.Error2(__METHOD__, sz.Text(), NULL);
		sz.Kill();
		return false;
	}
	return AddEvent(pcEvent);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false, without logging, if the event isn't after the current
// time.
//
//////////////////////////////////////////////////////////////////////////
bool CTimeline::AddEvent(CEvent* pcEvent)
{
	int64					iEventTime;
	CSimultaneousEvents*	pcEvents;
	CSimultaneousEvents		cNewEvents;

	iEventTime = pcEvent->GetTime();
	if (iEventTime <= miTime)
	{
		return false;
	}

	//The tree copies the new events into its node, which then owns them.
	pcEvents = mcEvents.Get(&iEventTime);
	if (pcEvents == NULL)
	{
		cNewEvents.Init(iEventTime);
		pcEvents = mcEvents.Add(&cNewEvents);
	}
	return pcEvents->Add(pcEvent);
}


//////////////////////////////////////////////////////////////////////////
//
// Runs every event, and every event they add, until there are none.
//
//////////////////////////////////////////////////////////////////////////
void CTimeline::Run(void)
{
	for (;;)
	{
		if (!RunSimultaneous())
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the events up to and including iTimeForward from now, then moves
// the time on to then.  Returns true if any events were run.
//
//////////////////////////////////////////////////////////////////////////
bool CTimeline::RunToTime(int64 iTimeForward)
{
	int64					iTargetTime;
	bool					bAnyProcessed;
	CSimultaneousEvents*	pcEvents;

	iTargetTime = miTime + iTimeForward;
	bAnyProcessed = false;

	for (;;)
	{
		pcEvents = mcEvents.GetFirst();
		if ((pcEvents != NULL) && (pcEvents->NumEvents() > 0))
		{
			if (pcEvents->GetTime() < miTime)
			{
				LogEventTimeError(__METHOD__, " Cannot update simulation time.  Event time [", pcEvents->GetTime());
				return bAnyProcessed;
			}

			if (pcEvents->GetTime() <= iTargetTime)
			{
				miTime = pcEvents->GetTime();
				ExecuteEvents(pcEvents);
				bAnyProcessed = true;
			}
			else
			{
				miTime = iTargetTime;
				break;
			}
		}
		else
		{
			miTime = iTargetTime;
			break;
		}
	}

	return bAnyProcessed;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the earliest events.  Returns false if there were none.
//
//////////////////////////////////////////////////////////////////////////
bool CTimeline::RunSimultaneous(void)
{
	CSimultaneousEvents*	pcEvents;

	pcEvents = mcEvents.GetFirst();
	if (pcEvents == NULL)
	{
		return false;
	}

	if (pcEvents->GetTime() < miTime)
	{
		LogEventTimeError(__METHOD__, " Cannot update simulation time.  Event time [", pcEvents->GetTime());
		return false;
	}

	miTime = pcEvents->GetTime();
	ExecuteEvents(pcEvents);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The events that were there when they started are executed in order,
// unless one of them has removed another.  None can be added, they would
// be in the past.  Then every one of them is freed.  Events added at
// later times while they run don't move them, the tree's nodes stay put.
//
//////////////////////////////////////////////////////////////////////////
void CTimeline::ExecuteEvents(CSimultaneousEvents* pcEvents)
{
	size		i;
	size		uiNumEvents;
	CEvent*		pcEvent;

	if (mpcRunning != NULL)
	{
		gcLogger.Error2(__METHOD__, " Events cannot be run while events are running.", NULL);
		return;
	}

	miPreviousEventTime = miEventTime;
	miEventTime = miTime;

	mpcRunning = pcEvents;
	uiNumEvents = pcEvents->NumEvents();
	for (i = 0; i < uiNumEvents; i++)
	{
		pcEvent = pcEvents->GetEvent(i);
		if (!pcEvent->IsRemoved())
		{
			pcEvent->Execute(this);
		}
	}
	mpcRunning = NULL;

	pcEvents->KillEvents();
	RemoveSimultaneousEvents(pcEvents);
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the event off the timeline, and from its owner, and frees it.
// One of the events executing now is only marked removed, it is freed
// when they have all run.  Returns false if the event isn't on the
// timeline; it isn't touched then.
//
//////////////////////////////////////////////////////////////////////////
bool CTimeline::Remove(CEvent* pcEvent)
{
	CSimultaneousEvents*	pcEvents;

	if ((mpcRunning != NULL) && mpcRunning->Contains(pcEvent))
	{
		if (!pcEvent->IsRemoved())
		{
			pcEvent->RemoveFromOwner();
		}
		return true;
	}

	pcEvents = FindSimultaneousEvents(pcEvent->GetTime());
	if ((pcEvents == NULL) || !pcEvents->Remove(pcEvent))
	{
		return false;
	}

	pcEvent->RemoveFromOwner();
	SafeKill(pcEvent);

	if (pcEvents->NumEvents() == 0)
	{
		RemoveSimultaneousEvents(pcEvents);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns true if any of the events were removed.
//
//////////////////////////////////////////////////////////////////////////
bool CTimeline::RemoveAll(CArrayEventPtr* papcEvents)
{
	size	i;
	bool	bResult;

	bResult = false;
	for (i = 0; i < papcEvents->NumElements(); i++)
	{
		bResult |= Remove(papcEvents->GetPtr(i));
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Kills the events at a time and takes them out of the tree.  Their
// events must already have been freed or taken out.
//
//////////////////////////////////////////////////////////////////////////
void CTimeline::RemoveSimultaneousEvents(CSimultaneousEvents* pcEvents)
{
	pcEvents->Kill();
	mcEvents.RemoveElement(pcEvents);
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if there are no events at iTime.
//
//////////////////////////////////////////////////////////////////////////
CSimultaneousEvents* CTimeline::FindSimultaneousEvents(int64 iTime)
{
	return mcEvents.Get(&iTime);
}


//////////////////////////////////////////////////////////////////////////
//
// The events at the earliest time, NULL if there are none.
//
//////////////////////////////////////////////////////////////////////////
CSimultaneousEvents* CTimeline::GetFirstSimultaneousEvents(void)
{
	return mcEvents.GetFirst();
}


//////////////////////////////////////////////////////////////////////////
//
// The events at the next time after pcEvents', NULL after the last.
//
//////////////////////////////////////////////////////////////////////////
CSimultaneousEvents* CTimeline::GetNextSimultaneousEvents(CSimultaneousEvents* pcEvents)
{
	return mcEvents.GetNext(pcEvents);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the red-black tree is consistent.
//
//////////////////////////////////////////////////////////////////////////
bool CTimeline::IsValid(void)
{
	return mcEvents.IsValid();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimeline::LogEventTimeError(const char* szMethod, const char* szMessage, int64 iEventTime)
{
	CChars	sz;

	sz.Init(szMessage);
	AppendNanoseconds(&sz, iEventTime);
	sz.Append("] must be after current time [");
	AppendNanoseconds(&sz, miTime);
	sz.Append("].");
	gcLogger.Error2(szMethod, sz.Text(), NULL);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimeline::Save(STimelineData* psData)
{
	psData->iTime = miTime;
	psData->iPreviousEventTime = miPreviousEventTime;
	psData->iEventTime = miEventTime;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTimeline::Load(STimelineData* psData)
{
	miTime = psData->iTime;
	miPreviousEventTime = psData->iPreviousEventTime;
	miEventTime = psData->iEventTime;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CTimeline::GetTime(void) { return miTime; }
int64 CTimeline::GetPreviousEventTime(void) { return miPreviousEventTime; }
int64 CTimeline::GetEventTime(void) { return miEventTime; }
size CTimeline::NumSimultaneousEvents(void) { return mcEvents.NumElements(); }
void* CTimeline::GetContext(void) { return mpvContext; }


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
#include "BaseLib/ConstructorCall.h"
#include "Timeline.h"
#include "IntegratedCircuit.h"
#include "TickEvent.h"


//////////////////////////////////////////////////////////////////////////
//
// Makes a tick iDelay from now and adds it to the timeline, which owns
// it.  NULL if it couldn't be added.
//
//////////////////////////////////////////////////////////////////////////
CTickEvent* CTickEvent::Schedule(int64 iDelay, CIntegratedCircuit* pcIntegratedCircuit, CTimeline* pcTimeline)
{
	CTickEvent*	pcEvent;

	pcEvent = NewMalloc<CTickEvent>();
	pcEvent->Init(pcTimeline->GetTime() + iDelay, pcIntegratedCircuit);
	if (pcTimeline->AddNewFutureEvent(pcEvent))
	{
		return pcEvent;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTickEvent::Execute(CTimeline* pcTimeline)
{
	CIntegratedCircuitEvent::Execute(pcTimeline);
	mpcIntegratedCircuit->ExecuteTick(pcTimeline);
}


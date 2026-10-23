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
#ifndef __INTEGRATED_CIRCUIT_H__
#define __INTEGRATED_CIRCUIT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Chars.h"
#include "Event.h"
#include "Pins.h"
#include "State.h"


//Logicim's IntegratedCircuit, with its Component interface: a chip with CPins on a CCircuit.  Subclasses create their
//state, start it when the simulation starts and respond when an input transitions; a clocked chip can schedule
//CTickEvents.
//
//A chip is made with NewMalloc; Init adds it to its circuit, which kills and frees it.  The chip owns its pins, which
//are made with NewMalloc too, and its state.  Killing a chip takes its events, and its ports' events, off the
//circuit's timeline and disconnects its ports.
//
//Logicim's IntegratedCircuit is generic over its pins and state; here a subclass casts GetPins and GetState.
class CCircuit;
class CIntegratedCircuitEvent;
class CIntegratedCircuit : public CConstructable
{
CONSTRUCTABLE(CIntegratedCircuit);
protected:
	CCircuit*		mpcCircuit;
	CPins*			mpcPins;
	CState*			mpcState;
	CChars			mszName;
	CArrayEventPtr	mapcEvents;

public:
			void			Init(CCircuit* pcCircuit, const char* szName, CPins* pcPins);
	virtual void			Kill(void);

	virtual void			ExecuteTick(CTimeline* pcTimeline);
	virtual void			TraceConnected(CTimeline* pcTimeline, CPort* pcPort);
	virtual void			Disconnect(CTimeline* pcTimeline);
	virtual void			Reset(CTimeline* pcTimeline);

	virtual CState*			CreateState(void) =0;
	virtual void			SimulationStarted(CTimeline* pcTimeline) =0;
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort) =0;
	virtual const char*		GetType(void) =0;

			bool			SetState(CState* pcState);
			CState*			GetState(void);
			bool			IsStateless(void);
			CPins*			GetPins(void);
			CPort*			GetPort(const char* szName);
			char*			GetName(void);
			CCircuit*		GetCircuit(void);
			void			AppendDescription(CChars* psz);

			float32			GetVCC(int64 iTime);
			float32			GetGND(int64 iTime);
			bool			IsPowered(int64 iTime);

			void			Add(CIntegratedCircuitEvent* pcEvent);
			bool			Remove(CIntegratedCircuitEvent* pcEvent);
			size			NumEvents(void);
			CEvent*			GetEvent(size uiIndex);

protected:
			void			ResetState(void);
};


typedef CArrayTemplatePtr<CIntegratedCircuit>	CArrayIntegratedCircuitPtr;


#endif // __INTEGRATED_CIRCUIT_H__


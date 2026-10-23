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
#ifndef __LOGIC_PORT_H__
#define __LOGIC_PORT_H__
#include "Port.h"
#include "PortOutput.h"
#include "LogicSignal.h"
#include "VoltageConfigurationSource.h"
#include "Event.h"


//Logicim's LogicPort: a chip's input or output on an analog CTrace.  Writing a value schedules a CSlewEvent after the
//family's hold time; when it executes the port slews its trace to the new voltage, schedules a CDriveEvent at the end
//of the slew, and the other logic ports on the trace schedule a CTransitionEvent for when the voltage crosses their
//input threshold.  A transition tells the chip, through its CPins, that an input has changed.  Reading compares the
//trace's voltage with the family's input thresholds.  The family's voltages scale with the chip's VCC.
//
//A logic port is a CLogicSignal so a chip's pins can be wired to it or to an ideal CMetaTrace.
//
//The port keeps its pending events in time order, see CPortEvent; the timeline owns them.  What it drives its trace
//with is a CPortOutput copied from its last executed output event, as the timeline frees the event.
class CPins;
class CPortEvent;
class CDriveEvent;
class CSlewEvent;
class CTransitionEvent;
typedef CArrayTemplatePtr<CPortEvent>	CArrayPortEventPtr;
class CLogicPort : public CPort, public CLogicSignal
{
CONSTRUCTABLE(CLogicPort);
protected:
	CVoltageConfigurationSource*	mpcVoltageConfigurationSource;
	CArrayPortEventPtr				mapcEvents;
	CPortOutput						mcOutput;

public:
			void							Init(EPortType eType, CPins* pcPins, const char* szName, CVoltageConfigurationSource* pcVoltageConfigurationSource);
	virtual void							Kill(void);

			void							Add(CPortEvent* pcEvent);
			void							Remove(CPortEvent* pcEvent);
			size							NumEvents(void);
			CPortEvent*						GetEvent(size uiIndex);

			void							WriteBool(CTimeline* pcTimeline, bool bValue);
	virtual void							Write(CTimeline* pcTimeline, bool bValue);
	virtual void							WriteImpedance(CTimeline* pcTimeline);
			STraceValue						ReadValue(int64 iTime);
	virtual STraceValue						Read(CTimeline* pcTimeline);

	virtual void							Disconnect(CTimeline* pcTimeline);
	virtual void							TraceConnected(CTimeline* pcTimeline);
	virtual void							Reset(void);
	virtual bool							IsLogicPort(void);
	virtual float32							GetVoltageOut(int64 iTime);

			void							DriveEvent(CTimeline* pcTimeline, CDriveEvent* pcDriveEvent);
			void							SlewEvent(CTimeline* pcTimeline, CSlewEvent* pcSlewEvent);
			bool							TraceSlew(CTimeline* pcTimeline, CPortOutput* pcSlew);
			void							TransitionEvent(CTimeline* pcTimeline, CTransitionEvent* pcEvent);
			void							GetFutureDriveEvents(int64 iTime, CArrayEventPtr* papcDriveEvents);

			CVoltageConfigurationSource*	GetVoltageConfigurationSource(void);
			CPortOutput*					GetOutput(void);
			float32							GetVCC(int64 iTime);
			float32							GetGND(int64 iTime);
			CPins*							GetPins(void);
			void							AppendEvents(CChars* psz);

protected:
			void							CreateTransitionEventFromExistingVoltage(CTimeline* pcTimeline);
			int64							CalculateLowTransitionTime(CPortOutput* pcSlew, float32 fStartVoltage, float32 fEndVoltage, float32 fLowVoltageIn);
			int64							CalculateHighTransitionTime(CPortOutput* pcSlew, float32 fStartVoltage, float32 fEndVoltage, float32 fHighVoltageIn);
			int64							CalculateTransitionTime(float32 fStartVoltage, float32 fEndVoltage, int64 iSlewTime, float32 fY);
};


#endif // __LOGIC_PORT_H__


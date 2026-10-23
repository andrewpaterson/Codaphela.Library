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
#ifndef __SLEW_EVENT_H__
#define __SLEW_EVENT_H__
#include "PortOutputEvent.h"


//Logicim's SlewEvent: a port starts slewing its trace to a new voltage.  The start voltage and slew time are worked
//out when it executes, from what the port was driving then and the family's slew rate, and a CDriveEvent at the end
//of the slew replaces any later drives.  The slew time is MAX_INT64 when the chip has no power.
class CTimeline;
class CDriveEvent;
class CVoltageConfigurationSource;
class CSlewEvent : public CPortOutputEvent
{
CONSTRUCTABLE(CSlewEvent);
protected:
	float32		mfStartVoltage;  //At the event's time.
	float32		mfEndVoltage;  //At the event's time plus the slew time.
	int64		miSlewTime;

public:
	static	CSlewEvent*		Schedule(CLogicPort* pcPort, float32 fEndVoltage, int64 iDelay, CTimeline* pcTimeline);

			void			Init(CLogicPort* pcPort, int64 iTime, float32 fEndVoltage);
			void			Init(CLogicPort* pcPort, int64 iTime, int64 iID, float32 fStartVoltage, float32 fEndVoltage, int64 iSlewTime);

	virtual void			Execute(CTimeline* pcTimeline);
			CDriveEvent*	Update(CTimeline* pcTimeline);

	virtual float32			GetVoltage(int64 iTime);
	virtual void			CopyOutput(CPortOutput* pcOutput);
			float32			CalculateVoltageAtTime(int64 iTime, float32 fStartVoltage);
			bool			ForTime(int64 iTime);
			int64			GetEndTime(void);
			float32			GetStartVoltage(void);
			float32			GetEndVoltage(void);
			int64			GetSlewTime(void);
	virtual void			PrintShort(CChars* psz);

protected:
			int64			CalculateSlewTime(CVoltageConfigurationSource* pcVoltageConfiguration, float32 fVcc);
			float32			CalculateStartVoltage(int64 iNowTime, CVoltageConfigurationSource* pcVoltageConfiguration, float32 fVcc);
};


#endif // __SLEW_EVENT_H__


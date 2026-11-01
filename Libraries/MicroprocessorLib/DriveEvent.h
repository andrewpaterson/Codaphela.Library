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
#ifndef __DRIVE_EVENT_H__
#define __DRIVE_EVENT_H__
#include "PortOutputEvent.h"


//Logicim's DriveEvent: a port drives its trace with a fixed voltage, or with NaN to stop driving it.
class CTimeline;
class CDriveEvent : public CPortOutputEvent
{
CONSTRUCTABLE(CDriveEvent);
protected:
	float32		mfVoltage;

public:
	static	CDriveEvent*	Schedule(CLogicPort* pcPort, int64 iDelay, float32 fVoltage, CTimeline* pcTimeline);

			void			Init(CLogicPort* pcPort, int64 iTime, float32 fVoltage);
			void			Init(CLogicPort* pcPort, int64 iTime, int64 iID, float32 fVoltage);

	virtual void			Execute(CTimeline* pcTimeline);
	virtual float32			GetVoltage(int64 iTime);
	virtual void			CopyOutput(CPortOutput* pcOutput);
	virtual bool			IsDriveEvent(void);
	virtual void			PrintShort(CChars* psz);
};


#endif // __DRIVE_EVENT_H__


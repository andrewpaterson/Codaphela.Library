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
#ifndef __PORT_EVENT_H__
#define __PORT_EVENT_H__
#include "Event.h"


//Logicim's PortEvent: an event on a CLogicPort.  The port keeps it in its list of events from Init until it executes
//or is removed.
class CLogicPort;
class CIntegratedCircuit;
class CPortEvent : public CEvent
{
CONSTRUCTABLE(CPortEvent);
protected:
	CLogicPort*		mpcPort;

public:
			void				Init(CLogicPort* pcPort, int64 iTime);
			void				Init(CLogicPort* pcPort, int64 iTime, int64 iID);

	virtual void				RemoveFromOwner(void);
	virtual bool				IsDriveEvent(void);
	virtual bool				IsOutputEvent(void);

			CLogicPort*			GetPort(void);
	virtual CIntegratedCircuit*	GetIntegratedCircuit(void);
			void				AppendPortOwner(CChars* psz);
};


#endif // __PORT_EVENT_H__


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
#ifndef __PORT_OUTPUT_EVENT_H__
#define __PORT_OUTPUT_EVENT_H__
#include "PortEvent.h"
#include "PortOutput.h"


//Logicim's PortOutputEvent: something a port drives its trace with.  When it executes the port copies its output.
class CPortOutputEvent : public CPortEvent
{
CONSTRUCTABLE(CPortOutputEvent);
public:
	virtual float32		GetVoltage(int64 iTime) =0;
	virtual void		CopyOutput(CPortOutput* pcOutput) =0;
	virtual bool		IsOutputEvent(void) { return true; }
};


#endif // __PORT_OUTPUT_EVENT_H__


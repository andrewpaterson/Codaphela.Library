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
#ifndef __LOGIC_PORT_BUS_H__
#define __LOGIC_PORT_BUS_H__
#include "LogicBus.h"


#define LOGIC_PORT_BUS_MAX_WIDTH	32


//A CLogicBus made of a CLogicPort per bit, the least significant first, so a chip's pins can drive a bus of analog
//traces.  A bit reads 1 only when its port reads high; low, unsettled and undriven bits read 0.  It doesn't own its
//ports.
class CLogicPort;
class CLogicPortBus : public CLogicBus
{
protected:
	CLogicPort*		mapcPorts[LOGIC_PORT_BUS_MAX_WIDTH];
	size			muiWidth;

public:
			void		Init(void);
			void		Kill(void);

			bool		Add(CLogicPort* pcPort);
			CLogicPort*	GetPort(size uiBit);

	virtual void		Write(CTimeline* pcTimeline, uint32 uiValue);
	virtual void		WriteImpedance(CTimeline* pcTimeline);
	virtual uint32		Read(CTimeline* pcTimeline);
	virtual size		GetWidth(void);
};


#endif // __LOGIC_PORT_BUS_H__


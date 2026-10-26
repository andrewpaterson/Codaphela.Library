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
#ifndef __PIN_H__
#define __PIN_H__
#include "Passive.h"
#include "TracePort.h"
#include "PowerInPort.h"


typedef CArrayTemplatePtr<CTracePort>	CArrayTracePortPtr;


//Logicim's Pin: a circuit's input or output, a trace port per bit, "Port 0" up, with VCC and GND as a chip has.
class CPin : public CPassive
{
CONSTRUCTABLE(CPin);
protected:
	CArrayTracePortPtr	mapcTracePorts;
	CPowerInPort*		mpcVCC;
	CPowerInPort*		mpcGND;

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, size uiPortCount);
			void			Class(void) override;
			void			Free(void) override;

	virtual const char*		GetType(void);
			size			NumTracePorts(void);
			CTracePort*		GetTracePort(size uiIndex);
			CPowerInPort*	GetVoltageCommon(void);
			CPowerInPort*	GetVoltageGround(void);
			float32			GetVCC(int64 iTime);
};


#endif // __PIN_H__


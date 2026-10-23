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
#ifndef __PINS_H__
#define __PINS_H__
#include "PortHolder.h"


//Logicim's Pins: the ports of a CIntegratedCircuit, always with a VCC and a GND power in port.  A chip's pins add
//their CLogicPorts in Init.  An input transition on a logic port is passed to the chip.
class CPowerInPort;
class CLogicPort;
class CIntegratedCircuit;
class CPins : public CPortHolder
{
protected:
	CIntegratedCircuit*		mpcIntegratedCircuit;
	CPowerInPort*			mpcVCC;
	CPowerInPort*			mpcGND;

public:
			void				Init(void);
	virtual void				Kill(void);

			void				SetIntegratedCircuit(CIntegratedCircuit* pcIntegratedCircuit);
			CIntegratedCircuit*	GetIntegratedCircuit(void);
			CPowerInPort*		GetVoltageCommon(void);
			CPowerInPort*		GetVoltageGround(void);

			void				InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual void				TraceConnected(CTimeline* pcTimeline, CPort* pcPort);
	virtual char*				GetName(void);
	virtual void				AppendDescription(CChars* psz);
	virtual CCircuitComponent*	GetComponent(void);
			const char*			GetType(void);

	static	void				CalculatePortName(CChars* psz, const char* szName, size i, size uiBufferCount);
};


#endif // __PINS_H__


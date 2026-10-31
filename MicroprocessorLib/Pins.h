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
#include "StandardLib/Object.h"
#include "PortHolder.h"


//Logicim's Pins: the ports of a CIntegratedCircuit, always with a VCC and a GND power in port.  A chip's pins add
//their CLogicPorts in Init.  An input transition on a logic port is passed to the chip.
//
//Pins are a CObject made with OMalloc and given to their chip's Init; the chip's pointer to them keeps them alive.
//They are freed with their chip, in either order, see CCircuitComponent.  Freeing them reaches the timeline through
//their chip, so nothing else may keep a Ptr to them: pass the OMalloc straight to the chip's OMalloc and get them back
//from the chip.
class CPowerInPort;
class CLogicPort;
class CIntegratedCircuit;
class CPins : public CObject, public CPortHolder
{
CONSTRUCTABLE(CPins);
protected:
	CIntegratedCircuit*		mpcIntegratedCircuit;
	CPowerInPort*			mpcVCC;
	CPowerInPort*			mpcGND;

public:
			void				Init(void);
			void				Class(void) override;
			void				Free(void) override;

			bool				Save(CObjectWriter* pcFile) override;
			bool				Load(CObjectReader* pcFile) override;

			void				SetIntegratedCircuit(CIntegratedCircuit* pcIntegratedCircuit);
			CIntegratedCircuit*	GetIntegratedCircuit(void);
			CPowerInPort*		GetVoltageCommon(void);
			CPowerInPort*		GetVoltageGround(void);

			void				InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual void				TraceConnected(CTimeline* pcTimeline, CPort* pcPort);
	virtual char*				GetComponentName(void);
	virtual void				AppendDescription(CChars* psz);
	virtual CCircuitComponent*	GetComponent(void);
			const char*			GetType(void);

	static	void				CalculatePortName(CChars* psz, const char* szName, size i, size uiBufferCount);
	static	void				CalculateGroupPortName(CChars* psz, size uiGroup, size uiGroupCount, const char* szName, size uiBit, size uiBitCount);
};


#endif // __PINS_H__


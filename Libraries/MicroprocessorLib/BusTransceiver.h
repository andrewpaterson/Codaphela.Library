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
#ifndef __BUS_TRANSCEIVER_H__
#define __BUS_TRANSCEIVER_H__
#include "IntegratedCircuit.h"
#include "GroupPins.h"


#define BUS_TRANSCEIVER_TYPE		"Bus Transceiver"


//The pins of a bus transceiver, in order.
enum EBusTransceiverRole
{
	BTR_A,
	BTR_B,
	BTR_Direction,
	BTR_OutputEnable,
};


//A three state transceiver between two buses, as a 74LVC245 is: "A0" up and "B0" up both ways, "DIR" and the active
//low "OE".  While output enable is low, a high direction drives each B bit from its A bit and a low one each A bit from
//its B bit; the other side doesn't drive, nor do both sides otherwise (an undriven enable or direction isn't
//asserted).  An undriven bit stops the bit it drives driving.  Its pins are a CGroupPins of GetRoles' roles.
class CBusTransceiver : public CIntegratedCircuit
{
CONSTRUCTABLE(CBusTransceiver);
public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins);

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			WriteOutputs(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);

			CGroupPins*		GetGroupPins(void);

	static	size			GetRoles(SGroupPinRole* asRoles, size uiBits);

protected:
			void			Drive(CTimeline* pcTimeline, size uiFrom, size uiTo, size uiBit);
};


#endif // __BUS_TRANSCEIVER_H__


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
#ifndef __BUS_BUFFER_H__
#define __BUS_BUFFER_H__
#include "IntegratedCircuit.h"
#include "GroupPins.h"


#define BUS_BUFFER_TYPE				"Bus Buffer"
#define BUS_BUFFER_MAX_ENABLES		2


//Groups of three state buffers each with its own active low output enables, as a 74LVC244 (two groups of four, one
//enable each), a 74LVC541 (one group of eight, two enables) and a 74AHC125 (four groups of one) are: "A0" up in, "OE"
//(or "OE1" and "OE2") and "Y0" up out.  While all a group's enables are low its outputs follow (or with bInverting
//are the opposite of) its inputs, and an undriven input stops its output driving; otherwise they don't drive (an
//undriven enable isn't asserted).  Its pins are a CGroupPins of GetRoles' roles: the inputs, the outputs then the
//enables.
class CBusBuffer : public CIntegratedCircuit
{
CONSTRUCTABLE(CBusBuffer);
protected:
	bool	mbInverting;

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins, bool bInverting);
			void			Class(void) override;

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			WriteOutputs(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);

			CGroupPins*		GetGroupPins(void);
			bool			IsInverting(void);

	static	size			GetRoles(SGroupPinRole* asRoles, size uiBits, size uiEnables);

protected:
			bool			IsEnabled(CTimeline* pcTimeline, size uiGroup);
			void			WriteGroupOutputs(CTimeline* pcTimeline, size uiGroup);
};


#endif // __BUS_BUFFER_H__


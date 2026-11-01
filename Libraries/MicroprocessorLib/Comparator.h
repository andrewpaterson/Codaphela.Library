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
#ifndef __COMPARATOR_H__
#define __COMPARATOR_H__
#include "IntegratedCircuit.h"
#include "GroupPins.h"


#define COMPARATOR_TYPE			"Comparator"


//The pins of an equality comparator, in order.
enum EComparatorRole
{
	CR_P,
	CR_Q,
	CR_Enable,
	CR_Equal,
};


//An equality comparator, as a 74LVC521 (eight bits) is: "P0" up and "Q0" up in, the active low enable "OE" and the
//active low "EQ" (P = Q) out.  Enabled, EQ is low while every P bit equals its Q bit; otherwise it's high.  An undriven
//input stops EQ driving; an undriven enable isn't asserted.  Its pins are a CGroupPins of GetRoles' roles.
class CComparator : public CIntegratedCircuit
{
CONSTRUCTABLE(CComparator);
public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins);

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			WriteOutputs(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);

			CGroupPins*		GetGroupPins(void);

	static	size			GetRoles(SGroupPinRole* asRoles, size uiBits);
};


#endif // __COMPARATOR_H__


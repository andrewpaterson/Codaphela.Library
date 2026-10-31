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
#ifndef __ADDER_H__
#define __ADDER_H__
#include "IntegratedCircuit.h"
#include "GroupPins.h"


#define ADDER_TYPE			"Adder"
#define ADDER_MAX_BITS		31


//The pins of an adder, in order.
enum EAdderRole
{
	AR_A,
	AR_B,
	AR_CarryIn,
	AR_Sum,
	AR_CarryOut,
};


//A binary full adder, as a 74LVC283 (four bits) is: "A0" up and "B0" up in with the carry in "CI", their sum "S0"
//up out with the carry out "CO".  An undriven input stops every output driving.  Its pins are a CGroupPins of
//GetRoles' roles.
class CAdder : public CIntegratedCircuit
{
CONSTRUCTABLE(CAdder);
public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins);

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			WriteOutputs(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);

			CGroupPins*		GetGroupPins(void);

	static	size			GetRoles(SGroupPinRole* asRoles, size uiBits);
};


#endif // __ADDER_H__


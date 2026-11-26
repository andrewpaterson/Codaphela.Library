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
#ifndef __ANALOG_COMPARATOR_H__
#define __ANALOG_COMPARATOR_H__
#include "IntegratedCircuit.h"
#include "AnalogComparatorPins.h"


#define ANALOG_COMPARATOR_TYPE	"Analog Comparator"


//An analog comparator, as a MAX9010: its output high while IN+ is above IN-, low while it is below, after its
//propagation delay.  It reads its inputs' voltages, not their levels (CAnalogComparatorInPort): a slewing input is a
//transition where it crosses the other's voltage.  Within the offset of each other, or either input undriven, the
//output stays as it was.  It remembers nothing beyond its output.
class CAnalogComparator : public CIntegratedCircuit
{
CONSTRUCTABLE(CAnalogComparator);
public:
			void					Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CAnalogComparatorPins> pPins);
	virtual void					SimulationStarted(CTimeline* pcTimeline);
	virtual void					InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*				GetType(void);
			CAnalogComparatorPins*	GetAnalogComparatorPins(void);
};


#endif // __ANALOG_COMPARATOR_H__


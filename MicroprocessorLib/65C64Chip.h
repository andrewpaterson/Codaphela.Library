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
#ifndef __65C64_CHIP_H__
#define __65C64_CHIP_H__
#include "IntegratedCircuit.h"
#include "65C64.h"
#include "65C64ChipPins.h"


#define C65C64_CHIP_TYPE	"65C64 Microprocessor"


//The 65C64 as a chip on analog traces, as CW65C816Chip is the W65C816S.  It owns the processor and the C65C64Pins it
//drives, wired to the chip's CLogicPorts, and passes every input transition to the processor.
class C65C64Chip : public CIntegratedCircuit
{
CONSTRUCTABLE(C65C64Chip);
protected:
	C65C64		mcMPU;
	C65C64Pins	mcSignals;

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, C65C64ChipPins* pcPins);
	virtual void			Kill(void);

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);
	virtual CState*			CreateState(void);

			C65C64*			GetMPU(void);
			C65C64ChipPins*	GetChipPins(void);
};


#endif // __65C64_CHIP_H__


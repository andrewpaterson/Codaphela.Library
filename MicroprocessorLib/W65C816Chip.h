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
#ifndef __W65C816_CHIP_H__
#define __W65C816_CHIP_H__
#include "IntegratedCircuit.h"
#include "W65C816.h"
#include "W65C816ChipPins.h"


#define W65C816_CHIP_TYPE	"W65C816 Microprocessor"


//Logicim's W65C816: the W65C816S as a chip on analog traces.  It owns the processor, a CW65C816, and the
//CW65C816Pins it drives, which are wired to the chip's CLogicPorts.  Every input transition is passed to the
//processor, which acts on PHI2's edges as it does under CW65C816Meta.  The processor holds its own state; Logicim's
//W65C816State is the processor here.
class CW65C816Chip : public CIntegratedCircuit
{
CONSTRUCTABLE(CW65C816Chip);
protected:
	CW65C816		mcMPU;
	CW65C816Pins	mcSignals;

public:
			void				Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CW65C816ChipPins> pPins);
			void				Class(void) override;
			void				Free(void) override;

	virtual void				SimulationStarted(CTimeline* pcTimeline);
	virtual void				InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*			GetType(void);
	virtual const char*			GetStateXMLName(void);
	virtual void				SaveXML(CXMLWriter* pcWriter);
	virtual bool				LoadXML(CMarkupTag* pcTag);

			CW65C816*			GetMPU(void);
			CW65C816ChipPins*	GetChipPins(void);
};


#endif // __W65C816_CHIP_H__


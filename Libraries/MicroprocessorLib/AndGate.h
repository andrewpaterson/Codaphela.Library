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
#ifndef __AND_GATE_H__
#define __AND_GATE_H__
#include "IntegratedCircuit.h"
#include "AndGatePins.h"
#include "AndLogic.h"


#define AND_GATE_TYPE	"AND Gate"
#define NAND_GATE_TYPE	"NAND Gate"


//Logicim's AndGate, see CAndLogic.  It owns its logic, made by CreateLogic.
class CAndGate : public CIntegratedCircuit
{
CONSTRUCTABLE(CAndGate);
protected:
	CAndLogic*	mpcLogic;

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CAndGatePins> pPins);
			void			Class(void) override;
			void			Free(void) override;

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);
			CAndGatePins*	GetAndGatePins(void);

protected:
	virtual CAndLogic*		CreateLogic(void);
};


//Logicim's NandGate.
class CNandGate : public CAndGate
{
CONSTRUCTABLE(CNandGate);
public:
	virtual const char*		GetType(void);

protected:
	virtual CAndLogic*		CreateLogic(void);
};


#endif // __AND_GATE_H__


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
#ifndef __GATE_ARRAY_H__
#define __GATE_ARRAY_H__
#include "IntegratedCircuit.h"
#include "GateArrayPins.h"


#define GATE_ARRAY_TYPE		"Gate Array"


//What each gate of a gate array does.
enum EGateFunction
{
	GF_And,
	GF_Nand,
	GF_Or,
	GF_Nor,
	GF_Xor,
	GF_Xnor,
	GF_Buffer,
	GF_Inverter,
};


//Gates that each do the same thing, as a 74 series part of them is: a quad 2-input NAND is four NANDs of two inputs.
//When powered each gate's output follows its own inputs, as the single gates' do (see CAndLogic, COrGate and
//CXorGate): an undriven input that decides the output stops it driving, and unsettled inputs are ignored.  A buffer
//or inverter has one input.
class CGateArray : public CIntegratedCircuit
{
CONSTRUCTABLE(CGateArray);
protected:
	EGateFunction	meFunction;

public:
			void				Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGateArrayPins> pPins, EGateFunction eFunction);
			void				Class(void) override;

	virtual void				SimulationStarted(CTimeline* pcTimeline);
	virtual void				InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*			GetType(void);

			CGateArrayPins*		GetGateArrayPins(void);
			EGateFunction		GetFunction(void);

	static	bool				IsNegated(EGateFunction eFunction);
	static	bool				IsSingleInput(EGateFunction eFunction);

protected:
			void				UpdateGate(CTimeline* pcTimeline, size uiGate);
};


#endif // __GATE_ARRAY_H__


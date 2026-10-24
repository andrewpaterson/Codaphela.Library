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
#ifndef __CONSTANT_H__
#define __CONSTANT_H__
#include "IntegratedCircuit.h"
#include "ConstantPins.h"


#define CONSTANT_TYPE	"Constant"


//Logicim's Constant: drives its outputs with the bits of a value, the least significant on "Output 0", when the
//simulation starts.  The value it drives starts as its default value and is set back to it by ResetState.
class CConstant : public CIntegratedCircuit
{
CONSTRUCTABLE(CConstant);
protected:
	int64	miDefaultValue;
	int64	miConstantValue;

public:
			void				Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CConstantPins* pcPins, int64 iDefaultValue);

	virtual void				SimulationStarted(CTimeline* pcTimeline);
	virtual void				InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*			GetType(void);
	virtual void				ResetState(void);
			CConstantPins*		GetConstantPins(void);
			int64				GetConstantValue(void);
			void				SetConstantValue(int64 iConstantValue);
};


#endif // __CONSTANT_H__


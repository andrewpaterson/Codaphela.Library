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
#ifndef __BUFFER_H__
#define __BUFFER_H__
#include "IntegratedCircuit.h"
#include "BufferPins.h"


#define BUFFER_TYPE		"Buffer"
#define INVERTER_TYPE	"Inverter"


//Logicim's Buffer: when powered each output follows its input.  An undriven input stops its output driving, see
//CAndLogic; an unsettled one leaves it alone.
class CBuffer : public CIntegratedCircuit
{
CONSTRUCTABLE(CBuffer);
public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CBufferPins* pcPins);

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);
			CBufferPins*	GetBufferPins(void);

protected:
	virtual bool			TransformOutput(bool bValue);
};


//Logicim's Inverter.
class CInverter : public CBuffer
{
CONSTRUCTABLE(CInverter);
public:
	virtual const char*		GetType(void);

protected:
	virtual bool			TransformOutput(bool bValue);
};


#endif // __BUFFER_H__


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
#ifndef __SUBCIRCUIT_INSTANCE_H__
#define __SUBCIRCUIT_INSTANCE_H__
#include "Pin.h"


struct SSubcircuitNamedPin
{
	CChars				szPinName;
	CArrayTracePortPtr	apcTracePorts;

	void	Init(const char* szName);
	void	Kill(void);
};


//The named pins are allocated one by one so the trace port arrays handed out don't move.
typedef CArrayTemplatePtr<SSubcircuitNamedPin>	CArraySubcircuitNamedPinPtr;


//Logicim's SubcircuitInstance: a subcircuit placed in a circuit.  Each of the subcircuit's pins is a list of trace
//ports, named by the pin, which join the traces outside to those inside.  Its CSubcircuitInstanceSimulation is where
//the subcircuit's own components are simulated.
class CSubcircuitInstanceSimulation;
class CSubcircuitInstance : public CPassive
{
CONSTRUCTABLE(CSubcircuitInstance);
protected:
	CArraySubcircuitNamedPinPtr		mapsNamedPins;
	CSubcircuitInstanceSimulation*	mpcSubcircuitInstanceSimulation;

public:
			void							Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName);
	virtual void							Kill(void);

	virtual const char*						GetType(void);
			CArrayTracePortPtr*				AddTracePorts(const char* szPinName, size uiPortCount);
			CArrayTracePortPtr*				GetTracePorts(const char* szPinName);
			size							NumNamedPins(void);
			char*							GetPinName(size uiIndex);

			CSubcircuitInstanceSimulation*	GetSubcircuitInstanceSimulation(void);
			void							SetSubcircuitInstanceSimulation(CSubcircuitInstanceSimulation* pcSubcircuitInstanceSimulation);

protected:
			SSubcircuitNamedPin*			FindNamedPin(const char* szPinName);
};


#endif // __SUBCIRCUIT_INSTANCE_H__


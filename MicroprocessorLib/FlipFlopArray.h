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
#ifndef __FLIP_FLOP_ARRAY_H__
#define __FLIP_FLOP_ARRAY_H__
#include "IntegratedCircuit.h"
#include "GroupPins.h"


#define FLIP_FLOP_ARRAY_TYPE		"D-type flip flop array"
#define FLIP_FLOP_ARRAY_MAX			8


//The pins of a flip flop array, in order.
enum EFlipFlopArrayRole
{
	FFAR_Clock,
	FFAR_Data,
	FFAR_Set,
	FFAR_Reset,
	FFAR_Q,
	FFAR_QB,
};


//D-type flip flops each with their own clock and active low set and reset, as a 74LVC74 (two) is: each one's "CP",
//"D", "SD" and "RD" in and "Q" and "QB" out, numbered "1CP" up when there's more than one.  Each behaves as a
//CDTypeFlipFlop does.  What each holds starts low and is what it remembers.  Its pins are a CGroupPins of GetRoles'
//roles.
class CFlipFlopArray : public CIntegratedCircuit
{
CONSTRUCTABLE(CFlipFlopArray);
protected:
	bool	mabStates[FLIP_FLOP_ARRAY_MAX];

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins);
			void			Class(void) override;

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			WriteOutputs(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);
	virtual const char*		GetStateXMLName(void);
	virtual void			SaveXML(CXMLWriter* pcWriter);
	virtual bool			LoadXML(CMarkupTag* pcTag);
	virtual void			ResetState(void);

			CGroupPins*		GetGroupPins(void);
			bool			GetState(size uiFlipFlop);
			void			SetState(size uiFlipFlop, bool bState);

	static	size			GetRoles(SGroupPinRole* asRoles);

protected:
			void			WriteFlipFlop(CTimeline* pcTimeline, size uiFlipFlop);
};


#endif // __FLIP_FLOP_ARRAY_H__


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
#ifndef __REGISTER_H__
#define __REGISTER_H__
#include "IntegratedCircuit.h"
#include "GroupPins.h"


#define REGISTER_TYPE			"Register"
#define LATCH_TYPE				"Latch"
#define REGISTER_MAX_GROUPS		4
#define REGISTER_MAX_BITS		32


//The pins of a register, in order.
enum ERegisterRole
{
	RR_Data,
	RR_Clock,
	RR_OutputEnable,
	RR_Output,
};


//Groups of bits each with its own clock and active low output enable, as a 74LVC574 (one group of eight) and a
//74LVC16374 (two) are: "D0" up in, "CP", "OE" and "Q0" up out.  A rising clock takes the data; a transparent register
//(a latch, as a 74LVC573) takes it while its latch enable ("LE" for "CP") is high and holds it when it goes low.  The
//outputs drive what's held while their output enable is low and stop driving otherwise (an undriven output enable
//isn't asserted).  A data bit that isn't high or low isn't taken.  What each group holds starts at zero and is what
//it remembers.  Its pins are a CGroupPins of GetRoles' roles.
class CRegister : public CIntegratedCircuit
{
CONSTRUCTABLE(CRegister);
protected:
	bool	mbTransparent;
	uint32	mauiValues[REGISTER_MAX_GROUPS];

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins, bool bTransparent);
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
			bool			IsTransparent(void);
			uint32			GetValue(size uiGroup);
			void			SetValue(size uiGroup, uint32 uiValue);

	static	size			GetRoles(SGroupPinRole* asRoles, size uiBits, bool bTransparent);

protected:
			void			TakeData(CTimeline* pcTimeline, size uiGroup);
			void			WriteGroupOutputs(CTimeline* pcTimeline, size uiGroup);
};


#endif // __REGISTER_H__


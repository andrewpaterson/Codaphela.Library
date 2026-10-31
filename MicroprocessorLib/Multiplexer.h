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
#ifndef __MULTIPLEXER_H__
#define __MULTIPLEXER_H__
#include "IntegratedCircuit.h"
#include "GroupPins.h"


#define MULTIPLEXER_TYPE				"Multiplexer"
#define MULTIPLEXER_MAX_SELECT_BITS		4


//The pins of a multiplexer, in order.
enum EMultiplexerRole
{
	MR_Input,
	MR_Output,
	MR_Select,
	MR_Enable,
};


//Channels that each select one of their inputs by shared select bits, as a 74LVC157 (four channels of two inputs) and a
//74LVC153 (two of four) are: each channel's "I0" up in and "Y" out, the shared "S" (or "S0" and "S1") and the active
//low "E", shared or a channel's own.  While enabled each channel's output follows the input its select bits choose;
//disabled it's low, or with bDisabledHighImpedance doesn't drive (as a bus switch's).  An undriven select or chosen
//input stops the output driving.  Its pins are a CGroupPins of GetRoles' roles.
class CMultiplexer : public CIntegratedCircuit
{
CONSTRUCTABLE(CMultiplexer);
protected:
	bool	mbDisabledHighImpedance;

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins, bool bDisabledHighImpedance);
			void			Class(void) override;

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			WriteOutputs(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);

			CGroupPins*		GetGroupPins(void);
			bool			IsDisabledHighImpedance(void);

	static	size			GetRoles(SGroupPinRole* asRoles, size uiSelectBits, bool bChannelEnables);

protected:
			void			WriteChannel(CTimeline* pcTimeline, size uiChannel);
};


#endif // __MULTIPLEXER_H__


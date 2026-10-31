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
#ifndef __DECODER_H__
#define __DECODER_H__
#include "IntegratedCircuit.h"
#include "GroupPins.h"


#define DECODER_TYPE				"Decoder"
#define DECODER_MAX_ADDRESS_BITS	5
#define DECODER_MAX_ENABLES			3


//The pins of a decoder, in order; its enables come after its outputs.
enum EDecoderRole
{
	DR_Address,
	DR_Output,
	DR_Enable,
};


//Groups that each decode their address to one of their active low outputs, as a 74LVC138 (one group of three address
//bits with two active low enables and one active high) and a 74LVC139 (two groups of two, one active low enable each)
//are: "A0" up in, "E" (or "E1" up, the active low first) and "Y0" up out.  Enabled, the output the address picks is
//low and the others high; disabled they're all high.  An undriven address bit stops the outputs driving; an undriven
//enable isn't asserted.  Its pins are a CGroupPins of GetRoles' roles.
class CDecoder : public CIntegratedCircuit
{
CONSTRUCTABLE(CDecoder);
protected:
	size	muiLowEnables;

public:
			void			Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, Ptr<CGroupPins> pPins, size uiLowEnables);
			void			Class(void) override;

	virtual void			SimulationStarted(CTimeline* pcTimeline);
	virtual void			WriteOutputs(CTimeline* pcTimeline);
	virtual void			InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort);
	virtual const char*		GetType(void);

			CGroupPins*		GetGroupPins(void);

	static	size			GetRoles(SGroupPinRole* asRoles, size uiAddressBits, size uiEnables);

protected:
			void			WriteGroup(CTimeline* pcTimeline, size uiGroup);
};


#endif // __DECODER_H__


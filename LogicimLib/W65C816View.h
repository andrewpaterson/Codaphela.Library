/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __W65C816_VIEW_H__
#define __W65C816_VIEW_H__
#include "IntegratedCircuitView.h"


enum EW65C816ViewDetail
{
	W65C816_OpCodeName,
	W65C816_OpCodeNumber,
	W65C816_Cycle,
	W65C816_Accumulator,
	W65C816_XIndex,
	W65C816_YIndex,
	W65C816_Stack,
	W65C816_DirectPage,
	W65C816_ProgramCounter,
	W65C816_DataBank,

	NUM_W65C816_VIEW_DETAILS
};


enum EW65C816ViewFlag
{
	W65C816_Emulation,
	W65C816_Carry,
	W65C816_Zero,
	W65C816_MemoryWidth,
	W65C816_IndexWidthOrBreak,
	W65C816_DecimalMode,
	W65C816_InterruptDisable,
	W65C816_Negative,
	W65C816_Overflow,

	NUM_W65C816_VIEW_FLAGS
};


//A W65C816 microprocessor, as Logicim's W65C816View: a 22 by 26 box with its inputs down the left (PHI2 with a
//clock notch) and its outputs and buses down the right, each labelled, and its registers and flags shown inside.  A
//flag is written in the text colour when it's set and the hidden text colour when it's clear.  In emulation mode the
//memory width flag is hidden and the index width flag is the break flag.
class CW65C816View : public CIntegratedCircuitView
{
CONSTRUCTABLE(CW65C816View);
protected:
	size	mauiDetails[NUM_W65C816_VIEW_DETAILS];
	size	mauiFlagTexts[NUM_W65C816_VIEW_FLAGS];
	size	mauiFlagBoxes[NUM_W65C816_VIEW_FLAGS];

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts);

	virtual void					Paint(CCircuitPainter* pcPainter);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);

			char*					GetDetail(EW65C816ViewDetail eDetail);
			char*					GetFlagText(EW65C816ViewFlag eFlag);
			bool					IsFlagSet(EW65C816ViewFlag eFlag);
			bool					IsFlagVisible(EW65C816ViewFlag eFlag);

protected:
			void					AddFlag(EW65C816ViewFlag eFlag, const char* szName, float64 x, float64 y, bool bSet, bool bVisible);
			void					SetFlag(EW65C816ViewFlag eFlag, bool bSet);
			void					SetFlagVisible(EW65C816ViewFlag eFlag, bool bVisible);
};


#endif // __W65C816_VIEW_H__

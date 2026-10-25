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
#ifndef __65C64_VIEW_H__
#define __65C64_VIEW_H__
#include "IntegratedCircuitView.h"


enum E65C64ViewDetail
{
	C65C64_OpCodeName,
	C65C64_OpCodeNumber,
	C65C64_Cycle,
	C65C64_A,
	C65C64_B,
	C65C64_X,
	C65C64_Y,
	C65C64_Stack,
	C65C64_V,
	C65C64_W,
	C65C64_ProgramCounter,
	C65C64_Widths,
	C65C64_InterruptDepth,

	NUM_65C64_VIEW_DETAILS
};


enum E65C64ViewFlag
{
	C65C64_EdgeDisable,
	C65C64_LevelDisable,
	C65C64_FlagF,
	C65C64_FlagG,
	C65C64_Negative,
	C65C64_Overflow,
	C65C64_Zero,
	C65C64_Carry,

	NUM_65C64_VIEW_FLAGS
};


//A 65C64 microprocessor, laid out as CW65C816View: a 36 by 32 box with its inputs down the left (PHI2 with a clock
//notch) and its outputs and buses down the right, each labelled, and its registers and flags shown inside.  The A, B, X
//and Y registers are shown in full (64 bits) whatever their width; the widths are a detail of their own.  A flag is
//written in the text colour when it's set and the hidden text colour when it's clear.
class C65C64View : public CIntegratedCircuitView
{
CONSTRUCTABLE(C65C64View);
protected:
	size	mauiDetails[NUM_65C64_VIEW_DETAILS];
	size	mauiFlagTexts[NUM_65C64_VIEW_FLAGS];

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts);

	virtual void					Paint(CCircuitPainter* pcPainter);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);

			char*					GetDetail(E65C64ViewDetail eDetail);
			bool					IsFlagSet(E65C64ViewFlag eFlag);

protected:
			void					AddFlag(E65C64ViewFlag eFlag, const char* szName, float64 x, float64 y);
			void					SetFlag(E65C64ViewFlag eFlag, bool bSet);
			void					SetDetail(E65C64ViewDetail eDetail, CChars* psz);
};


#endif // __65C64_VIEW_H__

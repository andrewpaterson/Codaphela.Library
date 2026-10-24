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
#include "BaseLib/ConstructorCall.h"
#include "MicroprocessorLib/65C64Chip.h"
#include "65C64View.h"


#define C65C64_VIEW_FONT_SIZE	10.0


//////////////////////////////////////////////////////////////////////////
//
// "0x" and the value's iNumBytes bytes in hex, most significant first.
//
//////////////////////////////////////////////////////////////////////////
static void Append65C64Hex(CChars* psz, uint64 uiValue, size iNumBytes)
{
	size	i;
	uint8	uiByte;

	psz->Append("0x");
	for (i = iNumBytes; i > 0; i--)
	{
		uiByte = (uint8)(uiValue >> ((i - 1) * 8));
		psz->AppendHexHiLo(&uiByte, 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The details and flags run down the middle (grid x) of the box from the
// title, facing North, as CW65C816View's.
//
//////////////////////////////////////////////////////////////////////////
void C65C64View::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts)
{
	SPortViewCreator	asBottom[] = {	{ "PHI2", "PHI2", 1, true, false },
										{ "RESB", "RESB", 1, false, true },
										{ "EIRQB0", "EIRQB 0", 1, false, true },
										{ "EIRQB1", "EIRQB 1", 1, false, true },
										{ "EIRQB2", "EIRQB 2", 1, false, true },
										{ "EIRQB3", "EIRQB 3", 1, false, true },
										{ "LIRQB", "LIRQB", 1, false, true } };
	SPortViewCreator	asTop[] = {		{ "RWB", "RWB", 1, false, false },
										{ "OPV", "OPV", 1, false, false },
										{ "VPA", "VPA", 1, false, false },
										{ "VDA", "VDA", 1, false, false },
										{ "VPB", "VPB", 1, false, true },
										{ "LB", "Lower Byte", 1, false, false },
										{ "UB", "Upper Byte", 1, false, false },
										{ "Address", "Address", 32, false, false },
										{ "Data", "Data", 16, false, false } };
	float64				fFlag;

	CIntegratedCircuitView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);

	mcShapes.AddRectangle(-16, -18, 16, 18, true, true);
	CreatePowerPortViews();
	CreateLabelledPortViews(asBottom, 7, asTop, 9, 18, -18, -6, -8, 2);

	mcShapes.AddText(-15, 0, "Microprocessor", CF_SansSerifBold, C65C64_VIEW_FONT_SIZE, TA_Centre);
	mauiDetails[C65C64_OpCodeName] = AddDetail(-13, 5, "Op-code", "RES");
	mauiDetails[C65C64_OpCodeNumber] = AddDetail(-11, 5, "Op-code", "0x0000");
	mauiDetails[C65C64_Cycle] = AddDetail(-9, 3, "Cycle", "0");
	mauiDetails[C65C64_A] = AddDetail(-7, 12, "A", "0x0000000000000000");
	mauiDetails[C65C64_B] = AddDetail(-5, 12, "B", "0x0000000000000000");
	mauiDetails[C65C64_X] = AddDetail(-3, 12, "X", "0x0000000000000000");
	mauiDetails[C65C64_Y] = AddDetail(-1, 12, "Y", "0x0000000000000000");
	mauiDetails[C65C64_Stack] = AddDetail(1, 7, "Stack", "0x0001ffff");
	mauiDetails[C65C64_V] = AddDetail(3, 7, "V", "0x00000000");
	mauiDetails[C65C64_W] = AddDetail(5, 7, "W", "0x00000000");
	mauiDetails[C65C64_ProgramCounter] = AddDetail(7, 7, "P-Counter", "0x00000000");
	mauiDetails[C65C64_Widths] = AddDetail(9, 10, "Widths", "a8 b8 x8 y8");
	mauiDetails[C65C64_InterruptDepth] = AddDetail(11, 4, "Interrupts", "0");

	fFlag = 4.9;
	AddFlag(C65C64_EdgeDisable, "E", 13, fFlag);
	AddFlag(C65C64_LevelDisable, "L", 13, fFlag - 1.4);
	AddFlag(C65C64_FlagF, "F", 13, fFlag - 1.4 * 2);
	AddFlag(C65C64_FlagG, "G", 13, fFlag - 1.4 * 3);
	AddFlag(C65C64_Negative, "N", 13, fFlag - 1.4 * 4);
	AddFlag(C65C64_Overflow, "V", 13, fFlag - 1.4 * 5);
	AddFlag(C65C64_Zero, "Z", 13, fFlag - 1.4 * 6);
	AddFlag(C65C64_Carry, "C", 13, fFlag - 1.4 * 7);

	mcShapes.AddText(15, 0, "65C64", CF_SansSerifBold, C65C64_VIEW_FONT_SIZE, TA_Centre);
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// A flag's letter in a small box, clear.
//
//////////////////////////////////////////////////////////////////////////
void C65C64View::AddFlag(E65C64ViewFlag eFlag, const char* szName, float64 x, float64 y)
{
	SComponentShape*	psShape;

	psShape = mcShapes.AddRectangle(x - 0.6, y - 0.6, x + 0.6, y + 0.6, true, true);
	psShape->fLineWidth = 1;
	psShape->eFill = SC_Background;

	mauiFlagTexts[eFlag] = mcShapes.NumShapes();
	mcShapes.AddText(x, y, szName, CF_SansSerif, C65C64_VIEW_FONT_SIZE, TA_Centre);
	SetFlag(eFlag, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64View::SetFlag(E65C64ViewFlag eFlag, bool bSet)
{
	mcShapes.SetBorder(mauiFlagTexts[eFlag], bSet ? SC_Text : SC_HiddenText);
}


//////////////////////////////////////////////////////////////////////////
//
// Sets the detail's text and empties psz for the next.
//
//////////////////////////////////////////////////////////////////////////
void C65C64View::SetDetail(E65C64ViewDetail eDetail, CChars* psz)
{
	mcShapes.SetText(mauiDetails[eDetail], psz->Text());
	psz->ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The registers and flags are shown as they are once there's a
// microprocessor, and kept after.
//
//////////////////////////////////////////////////////////////////////////
void C65C64View::Paint(CCircuitPainter* pcPainter)
{
	C65C64*		pcMPU;
	CChars		sz;

	if (mpcComponent != NULL)
	{
		pcMPU = ((C65C64Chip*)mpcComponent)->GetMPU();

		sz.Init();
		pcMPU->GetMnemonicString(&sz);
		SetDetail(C65C64_OpCodeName, &sz);
		Append65C64Hex(&sz, pcMPU->GetOpcode(), 2);
		SetDetail(C65C64_OpCodeNumber, &sz);
		sz.Append((int32)pcMPU->GetCycle());
		SetDetail(C65C64_Cycle, &sz);
		Append65C64Hex(&sz, pcMPU->GetA(), 8);
		SetDetail(C65C64_A, &sz);
		Append65C64Hex(&sz, pcMPU->GetB(), 8);
		SetDetail(C65C64_B, &sz);
		Append65C64Hex(&sz, pcMPU->GetX(), 8);
		SetDetail(C65C64_X, &sz);
		Append65C64Hex(&sz, pcMPU->GetY(), 8);
		SetDetail(C65C64_Y, &sz);
		Append65C64Hex(&sz, pcMPU->GetS(), 4);
		SetDetail(C65C64_Stack, &sz);
		Append65C64Hex(&sz, pcMPU->GetV(), 4);
		SetDetail(C65C64_V, &sz);
		Append65C64Hex(&sz, pcMPU->GetW(), 4);
		SetDetail(C65C64_W, &sz);
		Append65C64Hex(&sz, pcMPU->GetProgramCounter(), 4);
		SetDetail(C65C64_ProgramCounter, &sz);
		sz.Append("a");
		sz.Append((uint32)(8 * pcMPU->GetRegisterBytes(R65_A)));
		sz.Append(" b");
		sz.Append((uint32)(8 * pcMPU->GetRegisterBytes(R65_B)));
		sz.Append(" x");
		sz.Append((uint32)(8 * pcMPU->GetRegisterBytes(R65_X)));
		sz.Append(" y");
		sz.Append((uint32)(8 * pcMPU->GetRegisterBytes(R65_Y)));
		SetDetail(C65C64_Widths, &sz);
		sz.Append(pcMPU->GetInterruptDepth());
		SetDetail(C65C64_InterruptDepth, &sz);
		sz.Kill();

		SetFlag(C65C64_EdgeDisable, pcMPU->IsFlagSet(STATUS65_EDGE_DISABLE));
		SetFlag(C65C64_LevelDisable, pcMPU->IsFlagSet(STATUS65_LEVEL_DISABLE));
		SetFlag(C65C64_FlagF, pcMPU->IsFlagSet(STATUS65_FLAG_F));
		SetFlag(C65C64_FlagG, pcMPU->IsFlagSet(STATUS65_FLAG_G));
		SetFlag(C65C64_Negative, pcMPU->IsFlagSet(STATUS65_NEGATIVE));
		SetFlag(C65C64_Overflow, pcMPU->IsFlagSet(STATUS65_OVERFLOW));
		SetFlag(C65C64_Zero, pcMPU->IsFlagSet(STATUS65_ZERO));
		SetFlag(C65C64_Carry, pcMPU->IsFlagSet(STATUS65_CARRY));
	}
	CIntegratedCircuitView::Paint(pcPainter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* C65C64View::GetType(void)
{
	return "65C64";
}


//////////////////////////////////////////////////////////////////////////
//
// XML names can't start with a digit.
//
//////////////////////////////////////////////////////////////////////////
const char* C65C64View::GetXMLName(void)
{
	return "MPU65C64";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64View::LoadXML(CMarkupTag* pcTag)
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;
	char*		szFamily;
	bool		bExplicitPowerPorts;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName) || !LoadXMLFamily(pcTag, &szFamily, &bExplicitPowerPorts))
	{
		return false;
	}
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// C65C64InstructionFactory must be initialised before the chip runs.
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* C65C64View::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	C65C64ChipPins*		pcPins;
	C65C64Chip*			pcChip;

	pcPins = NewMalloc<C65C64ChipPins>();
	pcPins->Init(pcFamily);
	pcChip = NewMalloc<C65C64Chip>();
	pcChip->Init(pcSimulation, mszName.Text(), pcPins);
	return pcChip;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* C65C64View::GetDetail(E65C64ViewDetail eDetail)
{
	return mcShapes.GetText(mauiDetails[eDetail]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool C65C64View::IsFlagSet(E65C64ViewFlag eFlag)
{
	return mcShapes.GetShape(mauiFlagTexts[eFlag])->eBorder == SC_Text;
}

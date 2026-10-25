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
#include "MicroprocessorLib/W65C816Chip.h"
#include "W65C816View.h"


#define W65C816_VIEW_FONT_SIZE	10.0


//////////////////////////////////////////////////////////////////////////
//
// The details and then the flags run down the middle of the box from the
// title.
//
//////////////////////////////////////////////////////////////////////////
void CW65C816View::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts)
{
	SPortViewCreator	asLeft[] = {		{ "PHI2", "PHI2", 1, true, false },
										{ "RESB", "RESB", 1, false, true },
										{ "NMIB", "NMIB", 1, false, true },
										{ "IRQB", "IRQB", 1, false, true },
										{ "ABORTB", "ABORTB", 1, false, true },
										{ "RDY", "RDY", 1, false, false },
										{ "BE", "BE", 1, false, false } };
	SPortViewCreator	asRight[] = {	{ "RWB", "RWB", 1, false, false },
										{ "VDA", "VDA", 1, false, false },
										{ "VPA", "VPA", 1, false, false },
										{ "VPB", "VPB", 1, false, true },
										{ "Address", "Address", 16, false, false },
										{ "MLB", "MLB", 1, false, true },
										{ "E", "E", 1, false, false },
										{ "MX", "MX", 1, false, false },
										{ "Data", "Data", 8, false, false } };
	float64				fFlag;

	CIntegratedCircuitView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);

	mcShapes.AddRectangle(-11, -13, 11, 13, true, true);
	CreatePowerPortViews();
	CreateLabelledPortViews(asLeft, 7, asRight, 9, -11, 11, -6, -8, 2);

	mcShapes.AddText(0, -12, "Microprocessor", CF_SansSerifBold, W65C816_VIEW_FONT_SIZE, TA_Centre);
	mauiDetails[W65C816_OpCodeName] = AddDetail(-10, 4, "Op-code", "RES");
	mauiDetails[W65C816_OpCodeNumber] = AddDetail(-8, 4, "Op-code", "###");
	mauiDetails[W65C816_Cycle] = AddDetail(-6, 2, "Cycle", "0");
	mauiDetails[W65C816_Accumulator] = AddDetail(-4, 5, "Accumulator", "0x0000");
	mauiDetails[W65C816_XIndex] = AddDetail(-2, 5, "X Index", "0x0000");
	mauiDetails[W65C816_YIndex] = AddDetail(0, 5, "Y Index", "0x0000");
	mauiDetails[W65C816_Stack] = AddDetail(2, 5, "Stack", "0x01ff");
	mauiDetails[W65C816_DirectPage] = AddDetail(4, 5, "Direct page", "0x0000");
	mauiDetails[W65C816_ProgramCounter] = AddDetail(6, 7, "P-Counter", "0x00:0000");
	mauiDetails[W65C816_DataBank] = AddDetail(8, 4, "Data Bank", "0x00");

	fFlag = 5.5;
	AddFlag(W65C816_Emulation, "E", -fFlag, 10, true, true);
	AddFlag(W65C816_Carry, "C", -fFlag + 1.4, 10, false, true);
	AddFlag(W65C816_Zero, "Z", -fFlag + 1.4 * 2, 10, false, true);
	AddFlag(W65C816_MemoryWidth, "M", -fFlag + 1.4 * 3, 10, false, false);
	AddFlag(W65C816_IndexWidthOrBreak, "B", -fFlag + 1.4 * 4, 10, false, true);
	AddFlag(W65C816_DecimalMode, "D", -fFlag + 1.4 * 5, 10, false, true);
	AddFlag(W65C816_InterruptDisable, "I", -fFlag + 1.4 * 6, 10, false, true);
	AddFlag(W65C816_Negative, "N", -fFlag + 1.4 * 7, 10, false, true);
	AddFlag(W65C816_Overflow, "V", -fFlag + 1.4 * 8, 10, false, true);

	mcShapes.AddText(0, 12, "W65C816", CF_SansSerifBold, W65C816_VIEW_FONT_SIZE, TA_Centre);
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// A flag's letter in a small box.
//
//////////////////////////////////////////////////////////////////////////
void CW65C816View::AddFlag(EW65C816ViewFlag eFlag, const char* szName, float64 x, float64 y, bool bSet, bool bVisible)
{
	SComponentShape*	psShape;

	mauiFlagBoxes[eFlag] = mcShapes.NumShapes();
	psShape = mcShapes.AddRectangle(x - 0.6, y - 0.6, x + 0.6, y + 0.6, true, true);
	psShape->fLineWidth = 1;
	psShape->eFill = SC_Background;

	mauiFlagTexts[eFlag] = mcShapes.NumShapes();
	mcShapes.AddText(x, y, szName, CF_SansSerif, W65C816_VIEW_FONT_SIZE, TA_Centre);
	SetFlag(eFlag, bSet);
	SetFlagVisible(eFlag, bVisible);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816View::SetFlag(EW65C816ViewFlag eFlag, bool bSet)
{
	mcShapes.SetBorder(mauiFlagTexts[eFlag], bSet ? SC_Text : SC_HiddenText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816View::SetFlagVisible(EW65C816ViewFlag eFlag, bool bVisible)
{
	mcShapes.SetVisible(mauiFlagTexts[eFlag], bVisible);
	mcShapes.SetVisible(mauiFlagBoxes[eFlag], bVisible);
}


//////////////////////////////////////////////////////////////////////////
//
// The registers and flags are shown as they are once there's a
// microprocessor, and kept after.
//
//////////////////////////////////////////////////////////////////////////
void CW65C816View::Paint(CCircuitPainter* pcPainter)
{
	CW65C816*			pcMPU;
	CChars				sz;
	bool				bEmulation;
	CCircuitComponent*	pcComponent;

	pcComponent = GetComponent(pcPainter);
	if (pcComponent != NULL)
	{
		pcMPU = ((CW65C816Chip*)pcComponent)->GetMPU();

		sz.Init();
		pcMPU->GetOpcodeMnemonicString(&sz);
		mcShapes.SetText(mauiDetails[W65C816_OpCodeName], sz.Text());
		sz.ReInit();
		pcMPU->GetOpcodeValueHex(&sz);
		mcShapes.SetText(mauiDetails[W65C816_OpCodeNumber], sz.Text());
		sz.ReInit();
		pcMPU->GetCycleString(&sz);
		mcShapes.SetText(mauiDetails[W65C816_Cycle], sz.Text());
		sz.ReInit();
		pcMPU->GetAccumulatorValueHex(&sz);
		mcShapes.SetText(mauiDetails[W65C816_Accumulator], sz.Text());
		sz.ReInit();
		pcMPU->GetXValueHex(&sz);
		mcShapes.SetText(mauiDetails[W65C816_XIndex], sz.Text());
		sz.ReInit();
		pcMPU->GetYValueHex(&sz);
		mcShapes.SetText(mauiDetails[W65C816_YIndex], sz.Text());
		sz.ReInit();
		pcMPU->GetStackValueHex(&sz);
		mcShapes.SetText(mauiDetails[W65C816_Stack], sz.Text());
		sz.ReInit();
		pcMPU->GetDirectPageValueHex(&sz);
		mcShapes.SetText(mauiDetails[W65C816_DirectPage], sz.Text());
		sz.ReInit();
		pcMPU->GetProgramCounterValueHex(&sz);
		mcShapes.SetText(mauiDetails[W65C816_ProgramCounter], sz.Text());
		sz.ReInit();
		pcMPU->GetDataBankValueHex(&sz);
		mcShapes.SetText(mauiDetails[W65C816_DataBank], sz.Text());
		sz.Kill();

		bEmulation = pcMPU->IsEmulation();
		SetFlag(W65C816_Emulation, bEmulation);
		SetFlag(W65C816_Carry, pcMPU->IsCarrySet());
		SetFlag(W65C816_Zero, pcMPU->IsZeroFlagSet());
		SetFlag(W65C816_DecimalMode, pcMPU->IsDecimal());
		SetFlag(W65C816_InterruptDisable, pcMPU->IsInterruptDisable());
		SetFlag(W65C816_Negative, pcMPU->IsNegativeSet());
		SetFlag(W65C816_Overflow, pcMPU->IsOverflowFlag());
		if (bEmulation)
		{
			mcShapes.SetText(mauiFlagTexts[W65C816_IndexWidthOrBreak], "B");
			SetFlag(W65C816_IndexWidthOrBreak, pcMPU->IsBreak());
			SetFlagVisible(W65C816_MemoryWidth, false);
		}
		else
		{
			mcShapes.SetText(mauiFlagTexts[W65C816_IndexWidthOrBreak], "X");
			SetFlag(W65C816_IndexWidthOrBreak, pcMPU->IsIndex8Bit());
			SetFlagVisible(W65C816_MemoryWidth, true);
			SetFlag(W65C816_MemoryWidth, pcMPU->IsMemory8Bit());
		}
	}
	CIntegratedCircuitView::Paint(pcPainter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CW65C816View::GetType(void)
{
	return "W65C816";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CW65C816View::GetXMLName(void)
{
	return "W65C816";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816View::LoadXML(CMarkupTag* pcTag)
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
// CW65C816InstructionFactory must be initialised before the chip runs.
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CW65C816View::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	CW65C816ChipPins*	pcPins;
	CW65C816Chip*		pcChip;

	pcPins = NewMalloc<CW65C816ChipPins>();
	pcPins->Init(pcFamily);
	pcChip = NewMalloc<CW65C816Chip>();
	pcChip->Init(pcSimulation, mszName.Text(), pcPins);
	return pcChip;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CW65C816View::GetDetail(EW65C816ViewDetail eDetail)
{
	return mcShapes.GetText(mauiDetails[eDetail]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CW65C816View::GetFlagText(EW65C816ViewFlag eFlag)
{
	return mcShapes.GetText(mauiFlagTexts[eFlag]);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816View::IsFlagSet(EW65C816ViewFlag eFlag)
{
	return mcShapes.GetShape(mauiFlagTexts[eFlag])->eBorder == SC_Text;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CW65C816View::IsFlagVisible(EW65C816ViewFlag eFlag)
{
	return mcShapes.GetShape(mauiFlagTexts[eFlag])->bVisible;
}

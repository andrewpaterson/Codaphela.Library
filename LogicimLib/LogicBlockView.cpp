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
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "MicroprocessorLib/GroupPins.h"
#include "MicroprocessorLib/Register.h"
#include "MicroprocessorLib/BusBuffer.h"
#include "MicroprocessorLib/BusTransceiver.h"
#include "MicroprocessorLib/Multiplexer.h"
#include "MicroprocessorLib/Decoder.h"
#include "MicroprocessorLib/Adder.h"
#include "MicroprocessorLib/Comparator.h"
#include "MicroprocessorLib/Counter.h"
#include "MicroprocessorLib/FlipFlopArray.h"
#include "ComponentProperties.h"
#include "LogicBlockView.h"


//The box's edges and the room above and below its pins for its titles.
#define LOGIC_BLOCK_LEFT		-5
#define LOGIC_BLOCK_RIGHT		5
#define LOGIC_BLOCK_MARGIN		3
#define LOGIC_BLOCK_MAX_ROLES	8


//Each kind's element name and title.  The counter and flip flops are only parts' (see CLogic74PackageView): the
//generic counter and flip flop are CCounterView and CDTypeFlipFlopView.
struct SLogicBlockKind
{
	const char*		szXMLName;
	const char*		szTitle;
};


static SLogicBlockKind gasLogicBlockKinds[LBK_NumKinds] =
{
	{ "Register",			"Register" },
	{ "Latch",				"Latch" },
	{ "BusBuffer",			"Bus Buffer" },
	{ "BusTransceiver",		"Bus Transceiver" },
	{ "Multiplexer",		"Multiplexer" },
	{ "Decoder",			"Decoder" },
	{ "Adder",				"Adder" },
	{ "Comparator",			"Comparator" },
	{ NULL,					"Counter" },
	{ NULL,					"Flip Flops" },
};


//////////////////////////////////////////////////////////////////////////
//
// A pin labelled as its port is named.
//
//////////////////////////////////////////////////////////////////////////
static void AddLogicBlockPin(CArrayLogicBlockPin* pasPins, const char* szLabel, const char* szPortName, bool bRight, int iRow, bool bInverted = false, bool bClock = false)
{
	SLogicBlockPin*		psPin;

	psPin = pasPins->Add();
	StrCpySafe(psPin->szLabel, szLabel, LOGIC_BLOCK_NAME_LENGTH);
	StrCpySafe(psPin->szPortName, szPortName, LOGIC_BLOCK_NAME_LENGTH);
	psPin->bRight = bRight;
	psPin->iRow = iRow;
	psPin->bInverted = bInverted;
	psPin->bClock = bClock;
}


//////////////////////////////////////////////////////////////////////////
//
// A pin of a group chip (see CGroupPins), labelled as its port is named.
//
//////////////////////////////////////////////////////////////////////////
static void AddLogicBlockGroupPin(CArrayLogicBlockPin* pasPins, int iGroup, int iGroups, const char* szName, int iBit, int iBits, bool bRight, int iRow, bool bInverted = false, bool bClock = false)
{
	CChars	sz;

	sz.Init();
	CPins::CalculateGroupPortName(&sz, iGroup, iGroups, szName, iBit, iBits);
	AddLogicBlockPin(pasPins, sz.Text(), sz.Text(), bRight, iRow, bInverted, bClock);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each group's data in and out side by side, then its clock (or latch
// enable) and output enable.
//
//////////////////////////////////////////////////////////////////////////
static void GetRegisterPins(SLogicBlockParams* psParams, bool bTransparent, CArrayLogicBlockPin* pasPins)
{
	int		g;
	int		i;
	int		iRow;

	iRow = 0;
	for (g = 0; g < psParams->iGroups; g++)
	{
		for (i = 0; i < psParams->iBits; i++)
		{
			AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "D", i, psParams->iBits, false, iRow + i);
			AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "Q", i, psParams->iBits, true, iRow + i);
		}
		AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, bTransparent ? "LE" : "CP", 0, 1, false, iRow + psParams->iBits + 1, false, !bTransparent);
		AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "OE", 0, 1, false, iRow + psParams->iBits + 2, true);
		iRow += psParams->iBits + 4;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each group's inputs and outputs side by side, then its enables.
//
//////////////////////////////////////////////////////////////////////////
static void GetBusBufferPins(SLogicBlockParams* psParams, CArrayLogicBlockPin* pasPins)
{
	int			g;
	int			i;
	int			iRow;
	int			iEnableRow;
	const char*	aszEnables[] = { "OE1", "OE2" };

	iRow = 0;
	for (g = 0; g < psParams->iGroups; g++)
	{
		for (i = 0; i < psParams->iBits; i++)
		{
			AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "A", i, psParams->iBits, false, iRow + i);
			AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "Y", i, psParams->iBits, true, iRow + i, psParams->bInverting);
		}
		iEnableRow = iRow + psParams->iBits + ((psParams->iBits > 1) ? 1 : 0);
		for (i = 0; i < psParams->iEnables; i++)
		{
			AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, (psParams->iEnables == 1) ? "OE" : aszEnables[i], 0, 1, false, iEnableRow + i, true);
		}
		iRow = iEnableRow + psParams->iEnables + 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The A side down the left and the B side down the right, then direction
// and output enable.
//
//////////////////////////////////////////////////////////////////////////
static void GetBusTransceiverPins(SLogicBlockParams* psParams, CArrayLogicBlockPin* pasPins)
{
	int		i;

	for (i = 0; i < psParams->iBits; i++)
	{
		AddLogicBlockGroupPin(pasPins, 0, 1, "A", i, psParams->iBits, false, i);
		AddLogicBlockGroupPin(pasPins, 0, 1, "B", i, psParams->iBits, true, i);
	}
	AddLogicBlockPin(pasPins, "DIR", "DIR", false, psParams->iBits + 1);
	AddLogicBlockPin(pasPins, "OE", "OE", false, psParams->iBits + 2, true);
}


//////////////////////////////////////////////////////////////////////////
//
// Each channel's inputs with its output opposite their middle, then its
// enable if it has its own, then the shared select bits and enable.
//
//////////////////////////////////////////////////////////////////////////
static void GetMultiplexerPins(SLogicBlockParams* psParams, CArrayLogicBlockPin* pasPins)
{
	int		g;
	int		i;
	int		iRow;
	int		iInputs;

	iInputs = 1 << psParams->iBits;
	iRow = 0;
	for (g = 0; g < psParams->iGroups; g++)
	{
		for (i = 0; i < iInputs; i++)
		{
			AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "I", i, iInputs, false, iRow + i);
		}
		AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "Y", 0, 1, true, iRow + (iInputs - 1) / 2);
		iRow += iInputs;
		if (psParams->bChannelEnables)
		{
			AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "E", 0, 1, false, iRow, true);
			iRow++;
		}
		iRow++;
	}
	for (i = 0; i < psParams->iBits; i++)
	{
		AddLogicBlockGroupPin(pasPins, 0, 1, "S", i, psParams->iBits, false, iRow + i);
	}
	if (!psParams->bChannelEnables)
	{
		AddLogicBlockPin(pasPins, "E", "E", false, iRow + psParams->iBits, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each group's address bits then its enables down the left, and its
// active low outputs down the right.
//
//////////////////////////////////////////////////////////////////////////
static void GetDecoderPins(SLogicBlockParams* psParams, CArrayLogicBlockPin* pasPins)
{
	int			g;
	int			i;
	int			iRow;
	int			iOutputs;
	int			iRows;
	const char*	aszEnables[] = { "E1", "E2", "E3" };

	iOutputs = 1 << psParams->iBits;
	iRow = 0;
	for (g = 0; g < psParams->iGroups; g++)
	{
		for (i = 0; i < psParams->iBits; i++)
		{
			AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "A", i, psParams->iBits, false, iRow + i);
		}
		for (i = 0; i < psParams->iEnables; i++)
		{
			AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, (psParams->iEnables == 1) ? "E" : aszEnables[i], 0, 1, false, iRow + psParams->iBits + 1 + i, i < psParams->iLowEnables);
		}
		for (i = 0; i < iOutputs; i++)
		{
			AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "Y", i, iOutputs, true, iRow + i, true);
		}
		iRows = psParams->iBits + 1 + psParams->iEnables;
		if (iOutputs > iRows)
		{
			iRows = iOutputs;
		}
		iRow += iRows + 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A then B down the left with the carry in below, and the sum down the
// right opposite A with the carry out opposite the carry in.  One based,
// as a 74LVC283's: "A1" up, "CIN", "S1" up and "COUT".
//
//////////////////////////////////////////////////////////////////////////
static void GetAdderPins(SLogicBlockParams* psParams, CArrayLogicBlockPin* pasPins)
{
	int		i;
	CChars	szLabel;
	CChars	szPort;
	int		iBase;

	iBase = psParams->bOneBased ? 1 : 0;
	for (i = 0; i < psParams->iBits; i++)
	{
		szPort.Init();
		CPins::CalculateGroupPortName(&szPort, 0, 1, "A", i, psParams->iBits);
		szLabel.Init("A");
		szLabel.Append(i + iBase);
		AddLogicBlockPin(pasPins, szLabel.Text(), szPort.Text(), false, i);
		szLabel.Kill();
		szPort.Kill();

		szPort.Init();
		CPins::CalculateGroupPortName(&szPort, 0, 1, "B", i, psParams->iBits);
		szLabel.Init("B");
		szLabel.Append(i + iBase);
		AddLogicBlockPin(pasPins, szLabel.Text(), szPort.Text(), false, psParams->iBits + 1 + i);
		szLabel.Kill();
		szPort.Kill();

		szPort.Init();
		CPins::CalculateGroupPortName(&szPort, 0, 1, "S", i, psParams->iBits);
		szLabel.Init("S");
		szLabel.Append(i + iBase);
		AddLogicBlockPin(pasPins, szLabel.Text(), szPort.Text(), true, i);
		szLabel.Kill();
		szPort.Kill();
	}
	AddLogicBlockPin(pasPins, psParams->bOneBased ? "CIN" : "CI", "CI", false, 2 * psParams->iBits + 2);
	AddLogicBlockPin(pasPins, psParams->bOneBased ? "COUT" : "CO", "CO", true, 2 * psParams->iBits + 2);
}


//////////////////////////////////////////////////////////////////////////
//
// P then Q down the left with the enable below, and P = Q out opposite
// the gap between them.
//
//////////////////////////////////////////////////////////////////////////
static void GetComparatorPins(SLogicBlockParams* psParams, CArrayLogicBlockPin* pasPins)
{
	int		i;

	for (i = 0; i < psParams->iBits; i++)
	{
		AddLogicBlockGroupPin(pasPins, 0, 1, "P", i, psParams->iBits, false, i);
		AddLogicBlockGroupPin(pasPins, 0, 1, "Q", i, psParams->iBits, false, psParams->iBits + 1 + i);
	}
	AddLogicBlockPin(pasPins, "OE", "OE", false, 2 * psParams->iBits + 2, true);
	AddLogicBlockPin(pasPins, "P=Q", "EQ", true, psParams->iBits, true);
}


//////////////////////////////////////////////////////////////////////////
//
// A 74LVC161's pins on a counter's ports (see CCounterPins).
//
//////////////////////////////////////////////////////////////////////////
static void GetCounterPins(SLogicBlockParams* psParams, CArrayLogicBlockPin* pasPins)
{
	int		i;
	CChars	szLabel;
	CChars	szPort;

	AddLogicBlockPin(pasPins, "MR", "Reset", false, 0, true);
	AddLogicBlockPin(pasPins, "CP", "Clock", false, 1, false, true);
	AddLogicBlockPin(pasPins, "PE", "Load", false, 2, true);
	AddLogicBlockPin(pasPins, "CEP", "Enable", false, 3);
	AddLogicBlockPin(pasPins, "CET", "Enable Terminal", false, 4);
	AddLogicBlockPin(pasPins, "TC", "Terminal Count", true, 4);
	for (i = 0; i < psParams->iBits; i++)
	{
		szLabel.Init("D");
		szLabel.Append(i);
		szPort.Init();
		CPins::CalculatePortName(&szPort, "Data", i, psParams->iBits);
		AddLogicBlockPin(pasPins, szLabel.Text(), szPort.Text(), false, 6 + i);
		szPort.Kill();
		szLabel.Kill();

		szLabel.Init("Q");
		szLabel.Append(i);
		szPort.Init();
		CPins::CalculatePortName(&szPort, "Count", i, psParams->iBits);
		AddLogicBlockPin(pasPins, szLabel.Text(), szPort.Text(), true, 6 + i);
		szPort.Kill();
		szLabel.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each flip flop's set, data, clock and reset, and its Q and inverted Q
// opposite its data and clock.
//
//////////////////////////////////////////////////////////////////////////
static void GetFlipFlopPins(SLogicBlockParams* psParams, CArrayLogicBlockPin* pasPins)
{
	int		g;
	int		iRow;
	CChars	szLabel;
	CChars	szPort;

	iRow = 0;
	for (g = 0; g < psParams->iGroups; g++)
	{
		AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "SD", 0, 1, false, iRow, true);
		AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "D", 0, 1, false, iRow + 1);
		AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "CP", 0, 1, false, iRow + 2, false, true);
		AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "RD", 0, 1, false, iRow + 3, true);
		AddLogicBlockGroupPin(pasPins, g, psParams->iGroups, "Q", 0, 1, true, iRow + 1);

		szLabel.Init();
		CPins::CalculateGroupPortName(&szLabel, g, psParams->iGroups, "Q", 0, 1);
		szPort.Init();
		CPins::CalculateGroupPortName(&szPort, g, psParams->iGroups, "QB", 0, 1);
		AddLogicBlockPin(pasPins, szLabel.Text(), szPort.Text(), true, iRow + 2, true);
		szPort.Kill();
		szLabel.Kill();
		iRow += 5;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pins a block of the kind has, labelled and on its chip's ports.
//
//////////////////////////////////////////////////////////////////////////
void CLogicBlockView::GetPins(ELogicBlockKind eKind, SLogicBlockParams* psParams, CArrayLogicBlockPin* pasPins)
{
	switch (eKind)
	{
	case LBK_Register:			GetRegisterPins(psParams, false, pasPins);	break;
	case LBK_Latch:				GetRegisterPins(psParams, true, pasPins);	break;
	case LBK_BusBuffer:			GetBusBufferPins(psParams, pasPins);			break;
	case LBK_BusTransceiver:	GetBusTransceiverPins(psParams, pasPins);	break;
	case LBK_Multiplexer:		GetMultiplexerPins(psParams, pasPins);		break;
	case LBK_Decoder:			GetDecoderPins(psParams, pasPins);			break;
	case LBK_Adder:				GetAdderPins(psParams, pasPins);			break;
	case LBK_Comparator:		GetComparatorPins(psParams, pasPins);		break;
	case LBK_Counter:			GetCounterPins(psParams, pasPins);			break;
	case LBK_FlipFlops:			GetFlipFlopPins(psParams, pasPins);			break;
	default:					break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The generic chip of the kind and size, with its pins.
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CLogicBlockView::CreateChip(ELogicBlockKind eKind, SLogicBlockParams* psParams, CSubcircuitSimulation* pcSimulation, const char* szName, CFamilyVoltageConfiguration* pcFamily)
{
	SGroupPinRole	asRoles[LOGIC_BLOCK_MAX_ROLES];
	size			uiRoles;

	switch (eKind)
	{
	case LBK_Register:
	case LBK_Latch:
		uiRoles = CRegister::GetRoles(asRoles, psParams->iBits, eKind == LBK_Latch);
		return &OMalloc<CRegister>(pcSimulation, szName, OMalloc<CGroupPins>((size)psParams->iGroups, asRoles, uiRoles, pcFamily), eKind == LBK_Latch);
	case LBK_BusBuffer:
		uiRoles = CBusBuffer::GetRoles(asRoles, psParams->iBits, psParams->iEnables);
		return &OMalloc<CBusBuffer>(pcSimulation, szName, OMalloc<CGroupPins>((size)psParams->iGroups, asRoles, uiRoles, pcFamily), psParams->bInverting);
	case LBK_BusTransceiver:
		uiRoles = CBusTransceiver::GetRoles(asRoles, psParams->iBits);
		return &OMalloc<CBusTransceiver>(pcSimulation, szName, OMalloc<CGroupPins>((size)1, asRoles, uiRoles, pcFamily));
	case LBK_Multiplexer:
		uiRoles = CMultiplexer::GetRoles(asRoles, psParams->iBits, psParams->bChannelEnables);
		return &OMalloc<CMultiplexer>(pcSimulation, szName, OMalloc<CGroupPins>((size)psParams->iGroups, asRoles, uiRoles, pcFamily), psParams->bDisabledHighImpedance);
	case LBK_Decoder:
		uiRoles = CDecoder::GetRoles(asRoles, psParams->iBits, psParams->iEnables);
		return &OMalloc<CDecoder>(pcSimulation, szName, OMalloc<CGroupPins>((size)psParams->iGroups, asRoles, uiRoles, pcFamily), (size)psParams->iLowEnables);
	case LBK_Adder:
		uiRoles = CAdder::GetRoles(asRoles, psParams->iBits);
		return &OMalloc<CAdder>(pcSimulation, szName, OMalloc<CGroupPins>((size)1, asRoles, uiRoles, pcFamily));
	case LBK_Comparator:
		uiRoles = CComparator::GetRoles(asRoles, psParams->iBits);
		return &OMalloc<CComparator>(pcSimulation, szName, OMalloc<CGroupPins>((size)1, asRoles, uiRoles, pcFamily));
	case LBK_Counter:
		return &OMalloc<CCounter>(pcSimulation, szName, OMalloc<CCounterPins>(pcFamily, (size)psParams->iBits), (size)psParams->iBits, (uint32)((1LL << psParams->iBits) - 1), psParams->bSynchronousLoad);
	case LBK_FlipFlops:
		uiRoles = CFlipFlopArray::GetRoles(asRoles);
		return &OMalloc<CFlipFlopArray>(pcSimulation, szName, OMalloc<CGroupPins>((size)psParams->iGroups, asRoles, uiRoles, pcFamily));
	default:
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What the editor places: eight bits, a quad 2 to 1 multiplexer, a 3 to 8
// decoder with two active low enables and one active high, a four bit
// adder.
//
//////////////////////////////////////////////////////////////////////////
void CLogicBlockView::GetDefaultParams(ELogicBlockKind eKind, SLogicBlockParams* psParams)
{
	memset(psParams, 0, sizeof(SLogicBlockParams));
	psParams->iBits = 8;
	psParams->iGroups = 1;
	psParams->iEnables = 1;
	switch (eKind)
	{
	case LBK_Multiplexer:
		psParams->iBits = 1;
		psParams->iGroups = 4;
		break;
	case LBK_Decoder:
		psParams->iBits = 3;
		psParams->iEnables = 3;
		psParams->iLowEnables = 2;
		break;
	case LBK_Adder:
	case LBK_Counter:
		psParams->iBits = 4;
		break;
	case LBK_FlipFlops:
		psParams->iGroups = 2;
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Into the ranges the kind's chip can be.
//
//////////////////////////////////////////////////////////////////////////
static int ClampLogicBlockInt(int iValue, int iMinimum, int iMaximum)
{
	if (iValue < iMinimum)
	{
		return iMinimum;
	}
	if (iValue > iMaximum)
	{
		return iMaximum;
	}
	return iValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogicBlockView::ClampParams(ELogicBlockKind eKind, SLogicBlockParams* psParams)
{
	switch (eKind)
	{
	case LBK_Multiplexer:
		psParams->iBits = ClampLogicBlockInt(psParams->iBits, 1, MULTIPLEXER_MAX_SELECT_BITS);
		break;
	case LBK_Decoder:
		psParams->iBits = ClampLogicBlockInt(psParams->iBits, 1, DECODER_MAX_ADDRESS_BITS);
		psParams->iEnables = ClampLogicBlockInt(psParams->iEnables, 0, DECODER_MAX_ENABLES);
		psParams->iLowEnables = ClampLogicBlockInt(psParams->iLowEnables, 0, psParams->iEnables);
		break;
	case LBK_Adder:
		psParams->iBits = ClampLogicBlockInt(psParams->iBits, 1, ADDER_MAX_BITS);
		break;
	case LBK_Counter:
		psParams->iBits = ClampLogicBlockInt(psParams->iBits, 1, 31);
		break;
	case LBK_BusBuffer:
		psParams->iEnables = ClampLogicBlockInt(psParams->iEnables, 1, BUS_BUFFER_MAX_ENABLES);
		psParams->iBits = ClampLogicBlockInt(psParams->iBits, 1, LOGIC_BLOCK_MAX_BITS);
		break;
	default:
		psParams->iBits = ClampLogicBlockInt(psParams->iBits, 1, LOGIC_BLOCK_MAX_BITS);
		break;
	}
	if (eKind == LBK_FlipFlops)
	{
		psParams->iGroups = ClampLogicBlockInt(psParams->iGroups, 1, FLIP_FLOP_ARRAY_MAX);
	}
	else
	{
		psParams->iGroups = ClampLogicBlockInt(psParams->iGroups, 1, LOGIC_BLOCK_MAX_GROUPS);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogicBlockView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, ELogicBlockKind eKind, SLogicBlockParams* psParams)
{
	CIntegratedCircuitView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);
	meKind = eKind;
	msParams = *psParams;
	ClampParams(meKind, &msParams);

	CreatePowerPortViews();
	CreateBlock(GetKindTitle(meKind), NULL);
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// The box, its pins a grid unit apart about the position, and its title
// inside its top (and subtitle inside its bottom).
//
//////////////////////////////////////////////////////////////////////////
void CLogicBlockView::CreateBlock(const char* szTitle, const char* szSubtitle)
{
	CArrayLogicBlockPin		asPins;
	size					i;
	SLogicBlockPin*			psPin;
	int						iRows;
	int32					iFirst;
	float64					fTop;
	float64					fBottom;
	SPortViewCreator		sCreator;
	CPortView*				pcPortView;
	int32					y;

	asPins.Init();
	GetPins(meKind, &msParams, &asPins);
	iRows = 1;
	for (i = 0; i < asPins.NumElements(); i++)
	{
		if (asPins.Get(i)->iRow + 1 > iRows)
		{
			iRows = asPins.Get(i)->iRow + 1;
		}
	}
	iFirst = -(iRows - 1) / 2;
	fTop = iFirst - LOGIC_BLOCK_MARGIN;
	fBottom = iFirst + iRows - 1 + LOGIC_BLOCK_MARGIN;
	mcShapes.AddRectangle(LOGIC_BLOCK_LEFT, fTop, LOGIC_BLOCK_RIGHT, fBottom, true, true);

	for (i = 0; i < asPins.NumElements(); i++)
	{
		psPin = asPins.Get(i);
		sCreator.szText = psPin->szLabel;
		sCreator.szName = psPin->szPortName;
		sCreator.iPortCount = 1;
		sCreator.bClock = psPin->bClock;
		sCreator.bInverted = psPin->bInverted;
		y = iFirst + psPin->iRow;
		if (psPin->bRight)
		{
			pcPortView = CreateLabelledPortView(&sCreator, LOGIC_BLOCK_RIGHT, y, ROT_East);
			mcShapes.AddText(LOGIC_BLOCK_RIGHT - 0.6, y, pcPortView->GetText(), CF_SansSerif, LOGIC_BLOCK_FONT_SIZE, TA_Right);
		}
		else
		{
			pcPortView = CreateLabelledPortView(&sCreator, LOGIC_BLOCK_LEFT, y, ROT_West);
			mcShapes.AddText(LOGIC_BLOCK_LEFT + (psPin->bClock ? 1.2 : 0.6), y, pcPortView->GetText(), CF_SansSerif, LOGIC_BLOCK_FONT_SIZE, TA_Left);
		}
	}
	asPins.Kill();

	if (!StrEmpty(szTitle))
	{
		mcShapes.AddText((LOGIC_BLOCK_LEFT + LOGIC_BLOCK_RIGHT) / 2.0, fTop + 1.2, szTitle, CF_SansSerifBold, LOGIC_BLOCK_FONT_SIZE, TA_Centre);
	}
	if (!StrEmpty(szSubtitle))
	{
		mcShapes.AddText((LOGIC_BLOCK_LEFT + LOGIC_BLOCK_RIGHT) / 2.0, fBottom - 1.2, szSubtitle, CF_SansSerifBold, LOGIC_BLOCK_FONT_SIZE, TA_Centre);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The port view labelled szLabel and inverted (a bubble) or not.  If none
// is, the only port view labelled szLabel.  NULL if there isn't one.
//
//////////////////////////////////////////////////////////////////////////
CPortView* CLogicBlockView::GetPortViewByLabel(const char* szLabel, bool bInverted)
{
	size		i;
	CPortView*	pcPortView;
	CPortView*	pcOnly;
	size		uiLabelled;

	pcOnly = NULL;
	uiLabelled = 0;
	for (i = 0; i < NumPortViews(); i++)
	{
		pcPortView = GetPortView(i);
		if (StringCompare(pcPortView->GetText(), szLabel) == 0)
		{
			if (pcPortView->IsInverting() == bInverted)
			{
				return pcPortView;
			}
			pcOnly = pcPortView;
			uiLabelled++;
		}
	}
	return (uiLabelled == 1) ? pcOnly : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CLogicBlockView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	return CreateChip(meKind, &msParams, pcSimulation, mszName.Text(), pcFamily);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CLogicBlockView::GetKindXMLName(ELogicBlockKind eKind)
{
	if ((eKind < 0) || (eKind >= LBK_NumKinds))
	{
		return NULL;
	}
	return gasLogicBlockKinds[eKind].szXMLName;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CLogicBlockView::GetKindTitle(ELogicBlockKind eKind)
{
	if ((eKind < 0) || (eKind >= LBK_NumKinds))
	{
		return NULL;
	}
	return gasLogicBlockKinds[eKind].szTitle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogicBlockView::FindKind(const char* szXMLName, ELogicBlockKind* peKind)
{
	int		i;

	for (i = 0; i < LBK_NumKinds; i++)
	{
		if ((gasLogicBlockKinds[i].szXMLName != NULL) && (StringCompare(gasLogicBlockKinds[i].szXMLName, szXMLName) == 0))
		{
			*peKind = (ELogicBlockKind)i;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CLogicBlockView::GetType(void) { return GetKindTitle(GetBlockKind()); }
const char* CLogicBlockView::GetXMLName(void) { return GetKindXMLName(GetBlockKind()); }
ELogicBlockKind CLogicBlockView::GetBlockKind(void) { return meKind; }
ELogicBlockKind CLogicBlockView::GetKind(void) { return meKind; }
SLogicBlockParams* CLogicBlockView::GetParams(void) { return &msParams; }


//////////////////////////////////////////////////////////////////////////
//
// The kind's size: its bits and groups, and a bus buffer's enables and
// inversion, a multiplexer's enables and a decoder's.
//
//////////////////////////////////////////////////////////////////////////
void CLogicBlockView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CIntegratedCircuitView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("bitWidth", (int32)msParams.iBits);
	pcWriter->Attribute("groups", (int32)msParams.iGroups);
	if ((meKind == LBK_BusBuffer) || (meKind == LBK_Decoder))
	{
		pcWriter->Attribute("enables", (int32)msParams.iEnables);
	}
	if (meKind == LBK_Decoder)
	{
		pcWriter->Attribute("lowEnables", (int32)msParams.iLowEnables);
	}
	if (meKind == LBK_BusBuffer)
	{
		pcWriter->Attribute("inverting", msParams.bInverting);
	}
	if (meKind == LBK_Multiplexer)
	{
		pcWriter->Attribute("channelEnables", msParams.bChannelEnables);
		pcWriter->Attribute("disabledHighImpedance", msParams.bDisabledHighImpedance);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The kind is the class's (see GetBlockKind).  Missing sizes are the
// defaults.
//
//////////////////////////////////////////////////////////////////////////
bool CLogicBlockView::LoadXML(CMarkupTag* pcTag)
{
	int32				x;
	int32				y;
	ERotation			eRotation;
	char*				szName;
	char*				szFamily;
	bool				bExplicitPowerPorts;
	ELogicBlockKind		eKind;
	SLogicBlockParams	sParams;
	int32				iValue;
	bool				bValue;

	eKind = GetBlockKind();
	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName) || !LoadXMLFamily(pcTag, &szFamily, &bExplicitPowerPorts))
	{
		return false;
	}
	GetDefaultParams(eKind, &sParams);
	if (pcTag->GetInt32Attribute("bitWidth", &iValue))
	{
		sParams.iBits = iValue;
	}
	if (pcTag->GetInt32Attribute("groups", &iValue))
	{
		sParams.iGroups = iValue;
	}
	if (pcTag->GetInt32Attribute("enables", &iValue))
	{
		sParams.iEnables = iValue;
	}
	if (pcTag->GetInt32Attribute("lowEnables", &iValue))
	{
		sParams.iLowEnables = iValue;
	}
	if (pcTag->GetBoolAttribute("inverting", &bValue))
	{
		sParams.bInverting = bValue;
	}
	if (pcTag->GetBoolAttribute("channelEnables", &bValue))
	{
		sParams.bChannelEnables = bValue;
	}
	if (pcTag->GetBoolAttribute("disabledHighImpedance", &bValue))
	{
		sParams.bDisabledHighImpedance = bValue;
	}
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, eKind, &sParams);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Its size after its name and power, as it's saved.
//
//////////////////////////////////////////////////////////////////////////
void CLogicBlockView::GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	CComponentView::GetProperties(pcProperties, psContext);
	AddPowerProperties(pcProperties, psContext);
	pcProperties->AddInteger("bitWidth", msParams.iBits, 1, LOGIC_BLOCK_MAX_BITS);
	pcProperties->AddInteger("groups", msParams.iGroups, 1, LOGIC_BLOCK_MAX_GROUPS);
	if ((meKind == LBK_BusBuffer) || (meKind == LBK_Decoder))
	{
		pcProperties->AddInteger("enables", msParams.iEnables, 0, DECODER_MAX_ENABLES);
	}
	if (meKind == LBK_Decoder)
	{
		pcProperties->AddInteger("lowEnables", msParams.iLowEnables, 0, DECODER_MAX_ENABLES);
	}
	if (meKind == LBK_BusBuffer)
	{
		pcProperties->AddBoolean("inverting", msParams.bInverting);
	}
	if (meKind == LBK_Multiplexer)
	{
		pcProperties->AddBoolean("channelEnables", msParams.bChannelEnables);
		pcProperties->AddBoolean("disabledHighImpedance", msParams.bDisabledHighImpedance);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogicBlockView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	SLogicBlockParams	sParams;

	if (GetBlockKind() != meKind)
	{
		GetDefaultParams(GetBlockKind(), &msParams);
	}
	sParams = msParams;
	sParams.iBits = (int)pcProperties->GetInteger("bitWidth", sParams.iBits);
	sParams.iGroups = (int)pcProperties->GetInteger("groups", sParams.iGroups);
	sParams.iEnables = (int)pcProperties->GetInteger("enables", sParams.iEnables);
	sParams.iLowEnables = (int)pcProperties->GetInteger("lowEnables", sParams.iLowEnables);
	sParams.bInverting = pcProperties->GetBoolean("inverting", sParams.bInverting);
	sParams.bChannelEnables = pcProperties->GetBoolean("channelEnables", sParams.bChannelEnables);
	sParams.bDisabledHighImpedance = pcProperties->GetBoolean("disabledHighImpedance", sParams.bDisabledHighImpedance);
	Init(x, y, eRotation, pcProperties->GetText("name", ""), pcProperties->GetText("family", ""), pcProperties->GetBoolean("explicitPowerPorts", mbExplicitPowerPorts), GetBlockKind(), &sParams);
	return true;
}


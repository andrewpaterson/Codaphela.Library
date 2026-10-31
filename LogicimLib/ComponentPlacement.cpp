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
#include <stdlib.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/StringHelper.h"
#include "AndGateView.h"
#include "OrGateView.h"
#include "BufferView.h"
#include "Logic74View.h"
#include "LogicBlockView.h"
#include "ComponentViewFactory.h"
#include "PowerSourceView.h"
#include "ValueView.h"
#include "FlipFlopView.h"
#include "CounterView.h"
#include "ClockView.h"
#include "W65C816View.h"
#include "65C64View.h"
#include "OscilloscopeView.h"
#include "SplitterView.h"
#include "TunnelView.h"
#include "LabelView.h"
#include "SubcircuitInstanceView.h"
#include "ComponentPlacement.h"


#define PLACED_FAMILY		"LVC"
#define PLACED_REFERENCE	"U"


//////////////////////////////////////////////////////////////////////////
//
// A gate of the class with iInputCount one bit inputs, of the placed
// family, with implicit power and the next free reference.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
static CComponentView* NewPlacedGate(int32 x, int32 y, ERotation eRotation, int iInputCount, CCircuitView* pcCircuit)
{
	M*		pcView;
	CChars	szName;

	szName.Init();
	GetFreeReference(&szName, pcCircuit, PLACED_REFERENCE);
	pcView = NewMalloc<M>();
	pcView->Init(x, y, eRotation, szName.Text(), PLACED_FAMILY, false, iInputCount, 1);
	szName.Kill();
	return pcView;
}


//////////////////////////////////////////////////////////////////////////
//
// A chip of the class, of the placed family, with implicit power and the
// next free reference.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
static CComponentView* NewPlacedChip(int32 x, int32 y, ERotation eRotation, CCircuitView* pcCircuit)
{
	M*		pcView;
	CChars	szName;

	szName.Init();
	GetFreeReference(&szName, pcCircuit, PLACED_REFERENCE);
	pcView = NewMalloc<M>();
	pcView->Init(x, y, eRotation, szName.Text(), PLACED_FAMILY, false);
	szName.Kill();
	return pcView;
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's ComponentNameHelper.getUniqueName over the views of the
// same element name: a name already used (by number) is given the next
// number after the largest used, so a second "Pin" is "Pin 1".  An empty
// name is left empty.
//
//////////////////////////////////////////////////////////////////////////
void GetUniqueComponentName(CChars* pszName, CCircuitView* pcCircuit, const char* szXMLName, const char* szName)
{
	CChars				szStem;
	int					iNumber;
	CChars				szOtherStem;
	int					iOtherNumber;
	size				i;
	CComponentView*		pcComponent;
	bool				bSameStem;
	bool				bExactNumber;
	int					iMaxNumber;

	pszName->Init(szName);
	pszName->StripWhitespace();
	if (pszName->Empty())
	{
		return;
	}

	SplitComponentName(pszName->Text(), &szStem, &iNumber);
	bSameStem = false;
	bExactNumber = false;
	iMaxNumber = 0;
	for (i = 0; i < pcCircuit->NumComponents(); i++)
	{
		pcComponent = pcCircuit->GetComponent(i);
		if (StringCompare(pcComponent->GetXMLName(), szXMLName) == 0)
		{
			SplitComponentName(pcComponent->GetName(), &szOtherStem, &iOtherNumber);
			if (szOtherStem.Equals(&szStem))
			{
				bSameStem = true;
				if (iOtherNumber == iNumber)
				{
					bExactNumber = true;
				}
				if (iOtherNumber > iMaxNumber)
				{
					iMaxNumber = iOtherNumber;
				}
			}
			szOtherStem.Kill();
		}
	}
	if (bSameStem && (bExactNumber || (iMaxNumber == 0)))
	{
		pszName->Set(szStem);
		pszName->Append(' ');
		pszName->Append(iMaxNumber + 1);
	}
	szStem.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A name's trailing number (0 if it has none) and what comes before it,
// trimmed.
//
//////////////////////////////////////////////////////////////////////////
void SplitComponentName(const char* szName, CChars* pszStem, int* piNumber)
{
	size	uiEnd;

	pszStem->Init(szName);
	pszStem->StripWhitespace();
	uiEnd = pszStem->Length();
	while ((uiEnd > 0) && (pszStem->Text()[uiEnd - 1] >= '0') && (pszStem->Text()[uiEnd - 1] <= '9'))
	{
		uiEnd--;
	}
	*piNumber = (uiEnd < pszStem->Length()) ? atoi(&pszStem->Text()[uiEnd]) : 0;
	pszStem->SetLength(uiEnd);
	pszStem->StripWhitespace();
}


//////////////////////////////////////////////////////////////////////////
//
// The prefix and the lowest number after it that no component in the
// circuit is named (U1, U2 and so on).
//
//////////////////////////////////////////////////////////////////////////
void GetFreeReference(CChars* psz, CCircuitView* pcCircuit, const char* szPrefix)
{
	size	i;
	size	uiPrefix;
	int		iNumber;
	int		iUsed;
	char*	szName;
	char*	szEnd;

	uiPrefix = StrLen(szPrefix);
	iUsed = 0;
	if (pcCircuit != NULL)
	{
		for (i = 0; i < pcCircuit->NumComponents(); i++)
		{
			szName = pcCircuit->GetComponent(i)->GetName();
			if ((szName != NULL) && (strncmp(szName, szPrefix, uiPrefix) == 0) && (szName[uiPrefix] >= '0') && (szName[uiPrefix] <= '9'))
			{
				iNumber = (int)strtol(&szName[uiPrefix], &szEnd, 10);
				if ((*szEnd == '\0') && (iNumber > iUsed))
				{
					iUsed = iNumber;
				}
			}
		}
	}
	psz->Append(szPrefix);
	psz->Append((int32)(iUsed + 1));
}


//////////////////////////////////////////////////////////////////////////
//
// A real 74 series part ("Logic74Package:00") or its first unit
// ("Logic74Unit:00"), of the placed family and with the next free
// reference.  The units placed after a unit (see NewNextUnit) take its
// reference, as one part's.
//
//////////////////////////////////////////////////////////////////////////
static CComponentView* NewPlacedLogic74View(const char* szXMLName, const char* szPartType, int32 x, int32 y, ERotation eRotation, CCircuitView* pcCircuit)
{
	CChars					szName;
	SLogic74PartType		sType;
	CLogic74PackageView*	pcPackage;
	CLogic74UnitView*		pcUnit;
	CComponentView*			pcView;

	if (!FindLogic74Part(&sType, szPartType))
	{
		return NULL;
	}

	szName.Init();
	GetFreeReference(&szName, pcCircuit, PLACED_REFERENCE);
	pcView = NULL;
	if (strncmp(szXMLName, "Logic74Package:", 15) == 0)
	{
		pcPackage = NewMalloc<CLogic74PackageView>();
		pcPackage->Init(x, y, eRotation, szName.Text(), PLACED_FAMILY, false, szPartType);
		pcView = pcPackage;
	}
	else if (strncmp(szXMLName, "Logic74Unit:", 12) == 0)
	{
		pcUnit = NewMalloc<CLogic74UnitView>();
		pcUnit->Init(x, y, eRotation, szName.Text(), PLACED_FAMILY, false, szPartType, 0);
		pcView = pcUnit;
	}
	szName.Kill();
	return pcView;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentView* NewPlacedComponentView(const char* szXMLName, int32 x, int32 y, ERotation eRotation, CCircuitView* pcCircuit, CCircuitFonts* pcFonts)
{
	CPinView*			pcPin;
	CConstantView*		pcConstant;
	CProbeView*			pcProbe;
	CGroundView*		pcGround;
	CPositivePowerView*	pcPower;
	CCounterView*		pcCounter;
	CClockView*			pcClock;
	COscilloscopeView*	pcScope;
	CSplitterView*		pcSplitter;
	CTunnelView*		pcTunnel;
	CLabelView*			pcLabel;
	CChars				szName;
	const char*			szPartType;
	ELogicBlockKind		eKind;
	SLogicBlockParams	sParams;
	CLogicBlockView*	pcBlock;

	szPartType = strchr(szXMLName, PLACED_PART_SEPARATOR);
	if (szPartType != NULL)
	{
		return NewPlacedLogic74View(szXMLName, szPartType + 1, x, y, eRotation, pcCircuit);
	}
	if (CLogicBlockView::FindKind(szXMLName, &eKind))
	{
		CLogicBlockView::GetDefaultParams(eKind, &sParams);
		szName.Init();
		GetFreeReference(&szName, pcCircuit, PLACED_REFERENCE);
		pcBlock = (CLogicBlockView*)NewComponentView(szXMLName);
		pcBlock->Init(x, y, eRotation, szName.Text(), PLACED_FAMILY, false, eKind, &sParams);
		szName.Kill();
		return pcBlock;
	}

	if (StringCompare(szXMLName, "AndGate") == 0)
	{
		return NewPlacedGate<CAndGateView>(x, y, eRotation, 2, pcCircuit);
	}
	else if (StringCompare(szXMLName, "NandGate") == 0)
	{
		return NewPlacedGate<CNandGateView>(x, y, eRotation, 2, pcCircuit);
	}
	else if (StringCompare(szXMLName, "OrGate") == 0)
	{
		return NewPlacedGate<COrGateView>(x, y, eRotation, 2, pcCircuit);
	}
	else if (StringCompare(szXMLName, "NorGate") == 0)
	{
		return NewPlacedGate<CNorGateView>(x, y, eRotation, 2, pcCircuit);
	}
	else if (StringCompare(szXMLName, "XorGate") == 0)
	{
		return NewPlacedGate<CXorGateView>(x, y, eRotation, 2, pcCircuit);
	}
	else if (StringCompare(szXMLName, "XnorGate") == 0)
	{
		return NewPlacedGate<CXnorGateView>(x, y, eRotation, 2, pcCircuit);
	}
	else if (StringCompare(szXMLName, "Buffer") == 0)
	{
		return NewPlacedGate<CBufferView>(x, y, eRotation, 1, pcCircuit);
	}
	else if (StringCompare(szXMLName, "Inverter") == 0)
	{
		return NewPlacedGate<CInverterView>(x, y, eRotation, 1, pcCircuit);
	}
	else if (StringCompare(szXMLName, "Ground") == 0)
	{
		pcGround = NewMalloc<CGroundView>();
		pcGround->Init(x, y, eRotation, "");
		return pcGround;
	}
	else if (StringCompare(szXMLName, "PositivePower") == 0)
	{
		pcPower = NewMalloc<CPositivePowerView>();
		pcPower->Init(x, y, eRotation, "", 3.3f);
		return pcPower;
	}
	else if (StringCompare(szXMLName, "Constant") == 0)
	{
		pcConstant = NewMalloc<CConstantView>();
		pcConstant->Init(x, y, eRotation, "", PLACED_FAMILY, false, 2, RADIX_Binary, pcFonts);
		return pcConstant;
	}
	else if (StringCompare(szXMLName, "Probe") == 0)
	{
		pcProbe = NewMalloc<CProbeView>();
		pcProbe->Init(x, y, eRotation, "", PLACED_FAMILY, false, 32, RADIX_Hexadecimal, pcFonts);
		return pcProbe;
	}
	else if (StringCompare(szXMLName, "Pin") == 0)
	{
		GetUniqueComponentName(&szName, pcCircuit, szXMLName, "Pin");
		pcPin = NewMalloc<CPinView>();
		pcPin->Init(x, y, eRotation, szName.Text(), PLACED_FAMILY, false, 1, RADIX_Binary, pcFonts);
		szName.Kill();
		return pcPin;
	}
	else if (StringCompare(szXMLName, "DTypeFlipFlop") == 0)
	{
		return NewPlacedChip<CDTypeFlipFlopView>(x, y, eRotation, pcCircuit);
	}
	else if (StringCompare(szXMLName, "Counter") == 0)
	{
		szName.Init();
		GetFreeReference(&szName, pcCircuit, PLACED_REFERENCE);
		pcCounter = NewMalloc<CCounterView>();
		pcCounter->Init(x, y, eRotation, szName.Text(), PLACED_FAMILY, false, 4, 0xf);
		szName.Kill();
		return pcCounter;
	}
	else if (StringCompare(szXMLName, "Clock") == 0)
	{
		szName.Init();
		GetFreeReference(&szName, pcCircuit, PLACED_REFERENCE);
		pcClock = NewMalloc<CClockView>();
		pcClock->Init(x, y, eRotation, szName.Text(), PLACED_FAMILY, false, 25000000.0, false);
		szName.Kill();
		return pcClock;
	}
	else if (StringCompare(szXMLName, "W65C816") == 0)
	{
		return NewPlacedChip<CW65C816View>(x, y, eRotation, pcCircuit);
	}
	else if (StringCompare(szXMLName, "MPU65C64") == 0)
	{
		return NewPlacedChip<C65C64View>(x, y, eRotation, pcCircuit);
	}
	else if (StringCompare(szXMLName, "Oscilloscope") == 0)
	{
		pcScope = NewMalloc<COscilloscopeView>();
		pcScope->Init(x, y, eRotation, "", PLACED_FAMILY, 2, 32, 30, 4, 6000000000.0);
		return pcScope;
	}
	else if (StringCompare(szXMLName, "Splitter") == 0)
	{
		pcSplitter = NewMalloc<CSplitterView>();
		pcSplitter->Init(x, y, eRotation, "", 2, 2, 2, SA_LeftHanded, -1);
		return pcSplitter;
	}
	else if (StringCompare(szXMLName, "Tunnel") == 0)
	{
		pcTunnel = NewMalloc<CTunnelView>();
		pcTunnel->Init(x, y, eRotation, "  ", false, pcFonts);
		return pcTunnel;
	}
	else if (StringCompare(szXMLName, "Label") == 0)
	{
		pcLabel = NewMalloc<CLabelView>();
		pcLabel->Init(x, y, eRotation, "  ", TA_Left, false, true, true, pcFonts);
		return pcLabel;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Measured with the fonts, named and commented nothing.
//
//////////////////////////////////////////////////////////////////////////
CComponentView* NewPlacedSubcircuitInstanceView(CCircuitView* pcSubcircuit, int32 x, int32 y, ERotation eRotation, CCircuitFonts* pcFonts)
{
	CSubcircuitInstanceView*	pcInstance;

	pcInstance = NewMalloc<CSubcircuitInstanceView>();
	pcInstance->Init(x, y, eRotation, "", "", pcSubcircuit, pcFonts);
	return pcInstance;
}


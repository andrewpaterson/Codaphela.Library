/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/Logger.h"
#include "LogicimLib/AndGateView.h"
#include "LogicimLib/OrGateView.h"
#include "LogicimLib/BufferView.h"
#include "LogicimLib/FlipFlopView.h"
#include "LogicimLib/PowerSourceView.h"
#include "LogicimLib/TunnelView.h"
#include "LogicimLib/ValueView.h"
#include "LogicimLib/SubcircuitInstanceView.h"
#include "LogicimLib/TraceAutoConnector.h"
#include "LogicimLib/ComponentShapes.h"
#include "KicadSchemLogicimConverter.h"


//The spreads the components are placed at, from KiCad's places, widest first.
static float64 gafKicadSchemLogicimSpreads[] = { 3.0, 2.5, 2.1, 1.8, 1.55, 1.35, 1.2, 1.1, 1.0 };


//Reference prefixes and library names of the symbols that are left out.
static const char* gaszKicadSchemSkippedReferences[] = { "R", "RN", "C", "L", "FB", "D", "LED", "H", "MH", "TP", "NT", "FID" };
static const char* gaszKicadSchemSkippedLibraryNames[] = { "MountingHole", "NetTie", "TestPoint", "PWR_FLAG", "Fiducial" };


//Pin names on logic parts that are their power.
static const char* gaszKicadSchemPowerPinNames[] = { "GND", "VCC", "VDD", "VSS", "VEE", "3V", "5V", "3V3", "V+", "V-", "NC", "N.C." };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::Init(CKicadSchemFile* pcFile, CKicadSchemNetlist* pcNetlist, CCircuitViews* pcCircuits, CCircuitFonts* pcFonts)
{
	mpcFile = pcFile;
	mpcNetlist = pcNetlist;
	mpcCircuits = pcCircuits;
	mpcFonts = pcFonts;
	mpcCircuit = NULL;
	masItems.Init();
	masTerminals.Init();
	masConnections.Init();
	mfSpread = 1.0;
	muiUnconnected = 0;
	muiGates = 0;
	muiPlaceholders = 0;
	muiPowers = 0;
	muiTunnels = 0;
	muiSkipped = 0;
	muiOverlaps = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The circuits keep what was made.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::Kill(void)
{
	masConnections.Kill();
	masTerminals.Kill();
	masItems.Kill();
	mpcCircuit = NULL;
	mpcFonts = NULL;
	mpcCircuits = NULL;
	mpcNetlist = NULL;
	mpcFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the circuit szTypeName in the circuits from the sheet.  The
// netlist must be built.  Fails if there is already a circuit of the type.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemLogicimConverter::Convert(const char* szTypeName)
{
	size	i;
	size	uiNumSpreads;
	size	uiFirstUnconnected;
	size	uiUnconnected;
	size	uiBest;

	mpcCircuit = mpcCircuits->Add(szTypeName);
	if (mpcCircuit == NULL)
	{
		return false;
	}

	for (i = 0; i < mpcFile->NumSymbols(); i++)
	{
		if (!AddSymbol(mpcFile->GetSymbol(i)))
		{
			return false;
		}
	}
	AddTunnels();
	AttachItems();
	AddConnections();

	//Spread out and then pulled in while it wires no worse.
	uiNumSpreads = sizeof(gafKicadSchemLogicimSpreads) / sizeof(float64);
	uiBest = 0;
	Place(gafKicadSchemLogicimSpreads[0]);
	uiFirstUnconnected = Route();
	for (i = 1; i < uiNumSpreads; i++)
	{
		Place(gafKicadSchemLogicimSpreads[i]);
		uiUnconnected = Route();
		if (uiUnconnected > uiFirstUnconnected)
		{
			break;
		}
		uiBest = i;
	}
	mfSpread = gafKicadSchemLogicimSpreads[uiBest];
	Place(mfSpread);
	muiUnconnected = Route();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemLogicimConverter::AddSymbol(CKicadSchemSymbol* pcSymbol)
{
	CKicadSchemLibSymbol*		pcLibSymbol;
	EKicadSchemLogicFunction	eFunction;
	CChars						szFamily;
	bool						bAdded;

	pcLibSymbol = mpcFile->GetLibSymbol(pcSymbol);
	if (pcLibSymbol == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Symbol [", pcSymbol->GetReference(), "] has no library symbol.", NULL);
	}
	if (pcLibSymbol->IsPower())
	{
		return AddPower(pcSymbol, pcLibSymbol);
	}
	if (IsSkippedSymbol(pcSymbol))
	{
		muiSkipped++;
		return true;
	}

	szFamily.Init();
	eFunction = ParseKicadSchemLogicFunction(pcLibSymbol->GetName(), &szFamily);
	if (eFunction == KSLF_None)
	{
		eFunction = ParseKicadSchemLogicFunction(pcSymbol->GetValue(), &szFamily);
	}
	bAdded = false;
	if (eFunction != KSLF_None)
	{
		bAdded = AddLogicParts(pcSymbol, pcLibSymbol, eFunction, szFamily.Text());
	}
	szFamily.Kill();
	if (bAdded)
	{
		return true;
	}
	return AddPlaceholder(pcSymbol, pcLibSymbol);
}


//////////////////////////////////////////////////////////////////////////
//
// A ground or positive supply with its port on the power symbol's pin.
// Power flags are left out.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemLogicimConverter::AddPower(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol)
{
	size					i;
	CKicadSchemLibPin*		pcPin;
	char*					szName;
	CGroundView*			pcGround;
	CPositivePowerView*		pcPositive;
	CComponentView*			pcView;
	SKicadSchemPoint		sPosition;
	size					uiItem;

	pcPin = NULL;
	for (i = 0; i < pcLibSymbol->NumPins(); i++)
	{
		if (pcLibSymbol->GetPin(i)->IsOn(pcSymbol->GetUnit(), pcSymbol->GetBodyStyle()))
		{
			pcPin = pcLibSymbol->GetPin(i);
			break;
		}
	}
	szName = (pcPin != NULL) ? CKicadSchemNetlist::GetGlobalPowerName(pcSymbol, pcLibSymbol, pcPin) : NULL;
	if ((pcPin == NULL) || StrEmpty(szName) || (StringCompare(szName, "PWR_FLAG") == 0) || (strstr(pcLibSymbol->GetName(), "PWR_FLAG") != NULL))
	{
		muiSkipped++;
		return true;
	}

	if (IsGroundName(szName))
	{
		pcGround = NewMalloc<CGroundView>();
		pcGround->Init(0, 0, GetKicadRotation(ROT_North, pcSymbol->GetAngle()), szName);
		pcView = pcGround;
	}
	else
	{
		pcPositive = NewMalloc<CPositivePowerView>();
		pcPositive->Init(0, 0, GetKicadRotation(ROT_South, pcSymbol->GetAngle()), szName, GetSupplyVoltage(szName));
		pcView = pcPositive;
	}

	mpcFile->GetPinPosition(&sPosition, pcSymbol, pcPin);
	uiItem = AddItem(pcView, &sPosition, pcView->GetPortView((size)0));
	AddPinTerminal(pcSymbol, pcPin, pcView->GetPortView((size)0), uiItem);
	muiPowers++;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The parts of a pin's name: the gate number it starts with and the
// letters after it, and whether it's negated (~{...}).
//
//////////////////////////////////////////////////////////////////////////
static void SplitKicadSchemLogicPinName(const char* szName, CChars* pszIndex, CChars* pszLetters, bool* pbNegated)
{
	CChars	szName2;
	size	i;

	szName2.Init(szName);
	*pbNegated = false;
	if (szName2.StartsWith("~{"))
	{
		*pbNegated = true;
		szName2.RemoveFromStart(2);
		if (szName2.EndsWith("}"))
		{
			szName2.RemoveFromEnd(1);
		}
	}
	else if (szName2.StartsWith("~") && (szName2.Length() > 1))
	{
		*pbNegated = true;
		szName2.RemoveFromStart(1);
	}

	for (i = 0; i < szName2.Length(); i++)
	{
		if ((szName2.GetChar(i) < '0') || (szName2.GetChar(i) > '9'))
		{
			break;
		}
	}
	pszIndex->Init(szName2.Text(), 0, i);
	pszLetters->Init(szName2.Text(), i);
	pszLetters->UpperCase();
	szName2.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsKicadSchemPowerPinName(const char* szName)
{
	size	i;

	for (i = 0; i < sizeof(gaszKicadSchemPowerPinNames) / sizeof(const char*); i++)
	{
		if (StringInsensitiveCompare(szName, gaszKicadSchemPowerPinNames[i]) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The pin types a gate's output can have.
//
//////////////////////////////////////////////////////////////////////////
static bool IsKicadSchemOutputPinType(EKicadSchemPinType eType)
{
	return (eType == KSPT_Output) || (eType == KSPT_TriState) || (eType == KSPT_OpenCollector) || (eType == KSPT_OpenEmitter);
}


//////////////////////////////////////////////////////////////////////////
//
// The way from the inputs to the output.
//
//////////////////////////////////////////////////////////////////////////
static ERotation GetKicadSchemFacing(SKicadSchemPoint* psFrom, SKicadSchemPoint* psTo)
{
	float64		fDX;
	float64		fDY;

	fDX = psTo->fX - psFrom->fX;
	fDY = psTo->fY - psFrom->fY;
	if (fabs(fDX) >= fabs(fDY))
	{
		return (fDX >= 0) ? ROT_East : ROT_West;
	}
	return (fDY > 0) ? ROT_South : ROT_North;
}


//Which Logicim flip flop port a KiCad flip flop pin is, by its letters.
struct SKicadSchemFlipFlopPin
{
	const char*		szLetters;
	bool			bNegated;
	const char*		szPortName;
};


static SKicadSchemFlipFlopPin gasKicadSchemFlipFlopPins[] =
{
	{ "CP", false, "CP" }, { "CLK", false, "CP" }, { "C", false, "CP" },
	{ "SD", true, "SD" }, { "S", true, "SD" }, { "PRE", true, "SD" }, { "SET", true, "SD" },
	{ "D", false, "D" },
	{ "RD", true, "RD" }, { "R", true, "RD" }, { "CLR", true, "RD" }, { "RESET", true, "RD" },
	{ "Q", false, "Q" },
	{ "Q", true, "QB" },
};


//////////////////////////////////////////////////////////////////////////
//
// The Logicim flip flop port for the pin, or NULL.
//
//////////////////////////////////////////////////////////////////////////
static const char* GetKicadSchemFlipFlopPortName(CChars* pszLetters, bool bNegated)
{
	size	i;

	for (i = 0; i < sizeof(gasKicadSchemFlipFlopPins) / sizeof(SKicadSchemFlipFlopPin); i++)
	{
		if (pszLetters->Equals(gasKicadSchemFlipFlopPins[i].szLetters))
		{
			//Set and reset are active low however their pins are drawn.
			if ((gasKicadSchemFlipFlopPins[i].szPortName[0] == 'Q') && (gasKicadSchemFlipFlopPins[i].bNegated != bNegated))
			{
				continue;
			}
			return gasKicadSchemFlipFlopPins[i].szPortName;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A gate or flip flop for each gate number of the unit's pins.  False
// (and nothing added) if the pins aren't named as a logic part's are.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemLogicimConverter::AddLogicParts(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol, EKicadSchemLogicFunction eFunction, const char* szFamily)
{
	CArrayChars							aszIndices;
	CArrayTemplatePtr<CKicadSchemLibPin>	apcPins;
	CArrayInt							aiGroups;
	CArrayChars							aszLetters;
	CArrayInt							aiNegated;
	CArrayKicadSchemPoint				asPositions;
	CArrayInt							aiInputs;
	size								i;
	size								j;
	size								k;
	size								uiGroup;
	CKicadSchemLibPin*					pcPin;
	CChars								szIndex;
	CChars								szLetters;
	CChars								szName;
	bool								bNegated;
	bool								bValid;
	int									iOutput;
	int									iInputs;
	SKicadSchemPoint					sPosition;
	SKicadSchemPoint					sCentre;
	SKicadSchemPoint					sInputs;
	ERotation							eRotation;
	CComponentView*						pcView;
	size								uiItem;
	const char*							szPortName;
	int									iIndex;
	bool								bHasCP;
	bool								bHasD;
	bool								bHasQ;
	bool								bUnnamed;

	aszIndices.Init();
	apcPins.Init();
	aiGroups.Init();
	aszLetters.Init();
	aiNegated.Init();
	asPositions.Init();
	for (i = 0; i < pcLibSymbol->NumPins(); i++)
	{
		pcPin = pcLibSymbol->GetPin(i);
		if (!pcPin->IsOn(pcSymbol->GetUnit(), pcSymbol->GetBodyStyle()) || (pcPin->GetType() == KSPT_PowerIn) || IsKicadSchemPowerPinName(pcPin->GetName()))
		{
			continue;
		}
		SplitKicadSchemLogicPinName(pcPin->GetName(), &szIndex, &szLetters, &bNegated);
		uiGroup = aszIndices.GetIndex(szIndex.Text());
		if (uiGroup == ARRAY_ELEMENT_NOT_FOUND)
		{
			aszIndices.Add(szIndex.Text());
			uiGroup = aszIndices.NumElements() - 1;
		}
		apcPins.Add(pcPin);
		aiGroups.Add((int)uiGroup);
		aszLetters.Add(szLetters.Text());
		aiNegated.Add(bNegated ? 1 : 0);
		mpcFile->GetPinPosition(&sPosition, pcSymbol, pcPin);
		asPositions.Add(&sPosition);
		szLetters.Kill();
		szIndex.Kill();
	}

	//Unnamed pins (KiCad's own 74xx symbols name their gates' pins "~") are the inputs, A up in pin number order, and
	//the output Y.
	bUnnamed = (eFunction != KSLF_DFlipFlop) && (apcPins.NumElements() > 0);
	for (i = 0; bUnnamed && (i < apcPins.NumElements()); i++)
	{
		bUnnamed = aszLetters.Get(i)->Empty() || aszLetters.Get(i)->Equals("~");
	}
	if (bUnnamed)
	{
		for (i = 0; i < apcPins.NumElements(); i++)
		{
			if (IsKicadSchemOutputPinType(apcPins.GetPtr(i)->GetType()))
			{
				aszLetters.Get(i)->Set("Y");
				continue;
			}
			k = 0;
			for (j = 0; j < apcPins.NumElements(); j++)
			{
				if ((j != i) && !IsKicadSchemOutputPinType(apcPins.GetPtr(j)->GetType()) &&
					((atoi(apcPins.GetPtr(j)->GetNumber()) < atoi(apcPins.GetPtr(i)->GetNumber())) || ((atoi(apcPins.GetPtr(j)->GetNumber()) == atoi(apcPins.GetPtr(i)->GetNumber())) && (j < i))))
				{
					k++;
				}
			}
			szLetters.Init();
			szLetters.Append((char)('A' + k));
			aszLetters.Get(i)->Set(szLetters.Text());
			szLetters.Kill();
		}
	}

	//Every gate needs an output and its inputs; a flip flop a clock, data and output.
	bValid = aszIndices.NumElements() > 0;
	for (uiGroup = 0; bValid && (uiGroup < aszIndices.NumElements()); uiGroup++)
	{
		iOutput = 0;
		iInputs = 0;
		bHasCP = false;
		bHasD = false;
		bHasQ = false;
		for (i = 0; i < apcPins.NumElements(); i++)
		{
			if (aiGroups.GetValue(i) != (int)uiGroup)
			{
				continue;
			}
			if (eFunction == KSLF_DFlipFlop)
			{
				szPortName = GetKicadSchemFlipFlopPortName(aszLetters.Get(i), aiNegated.GetValue(i) != 0);
				if (szPortName == NULL)
				{
					bValid = false;
				}
				else
				{
					bHasCP |= StringCompare(szPortName, "CP") == 0;
					bHasD |= StringCompare(szPortName, "D") == 0;
					bHasQ |= StringCompare(szPortName, "Q") == 0;
				}
			}
			else if (aszLetters.Get(i)->Equals("Y"))
			{
				iOutput++;
			}
			else if (aszLetters.Get(i)->Length() == 1)
			{
				iInputs++;
			}
			else
			{
				bValid = false;
			}
		}
		if (eFunction == KSLF_DFlipFlop)
		{
			bValid &= bHasCP && bHasD && bHasQ;
		}
		else if ((eFunction == KSLF_Inverter) || (eFunction == KSLF_Buffer))
		{
			bValid &= (iOutput == 1) && (iInputs == 1);
		}
		else
		{
			bValid &= (iOutput == 1) && (iInputs >= 2);
		}
	}

	for (uiGroup = 0; bValid && (uiGroup < aszIndices.NumElements()); uiGroup++)
	{
		//The inputs in letter order, then the output.
		aiInputs.Init();
		iOutput = -1;
		sCentre.Init();
		sInputs.Init();
		k = 0;
		for (i = 0; i < apcPins.NumElements(); i++)
		{
			if (aiGroups.GetValue(i) != (int)uiGroup)
			{
				continue;
			}
			sCentre.fX += asPositions.Get(i)->fX;
			sCentre.fY += asPositions.Get(i)->fY;
			k++;
			if ((eFunction != KSLF_DFlipFlop) && aszLetters.Get(i)->Equals("Y"))
			{
				iOutput = (int)i;
			}
			else if ((eFunction == KSLF_DFlipFlop) && (GetKicadSchemFlipFlopPortName(aszLetters.Get(i), aiNegated.GetValue(i) != 0)[0] == 'Q'))
			{
				if (StringCompare(GetKicadSchemFlipFlopPortName(aszLetters.Get(i), aiNegated.GetValue(i) != 0), "Q") == 0)
				{
					iOutput = (int)i;
				}
			}
			else
			{
				for (j = 0; j < aiInputs.NumElements(); j++)
				{
					if (strcmp(aszLetters.GetText(i), aszLetters.GetText(aiInputs.GetValue(j))) < 0)
					{
						break;
					}
				}
				aiInputs.InsertAt((int)i, j);
				sInputs.fX += asPositions.Get(i)->fX;
				sInputs.fY += asPositions.Get(i)->fY;
			}
		}
		sCentre.fX /= (float64)k;
		sCentre.fY /= (float64)k;
		sInputs.fX /= (float64)aiInputs.NumElements();
		sInputs.fY /= (float64)aiInputs.NumElements();
		eRotation = GetKicadSchemFacing(&sInputs, asPositions.Get(iOutput));

		szName.Init(pcSymbol->GetReference());
		if (aszIndices.Get(uiGroup)->Length() > 0)
		{
			iIndex = atoi(aszIndices.GetText(uiGroup));
			if ((iIndex >= 1) && (iIndex <= 26))
			{
				szName.Append((char)('A' + iIndex - 1));
			}
			else
			{
				szName.Append(aszIndices.GetText(uiGroup));
			}
		}
		else if (pcLibSymbol->NumUnits() > 1)
		{
			//A gate to a unit, as KiCad's U1A.
			szName.Append((char)('A' + pcSymbol->GetUnit() - 1));
		}

		iInputs = (int)aiInputs.NumElements();
		switch (eFunction)
		{
		case KSLF_And:
			pcView = NewMalloc<CAndGateView>();
			((CAndGateView*)pcView)->Init(0, 0, eRotation, szName.Text(), szFamily, false, iInputs, 1);
			break;
		case KSLF_Nand:
			pcView = NewMalloc<CNandGateView>();
			((CNandGateView*)pcView)->Init(0, 0, eRotation, szName.Text(), szFamily, false, iInputs, 1);
			break;
		case KSLF_Or:
			pcView = NewMalloc<COrGateView>();
			((COrGateView*)pcView)->Init(0, 0, eRotation, szName.Text(), szFamily, false, iInputs, 1);
			break;
		case KSLF_Nor:
			pcView = NewMalloc<CNorGateView>();
			((CNorGateView*)pcView)->Init(0, 0, eRotation, szName.Text(), szFamily, false, iInputs, 1);
			break;
		case KSLF_Xor:
			pcView = NewMalloc<CXorGateView>();
			((CXorGateView*)pcView)->Init(0, 0, eRotation, szName.Text(), szFamily, false, iInputs, 1);
			break;
		case KSLF_Xnor:
			pcView = NewMalloc<CXnorGateView>();
			((CXnorGateView*)pcView)->Init(0, 0, eRotation, szName.Text(), szFamily, false, iInputs, 1);
			break;
		case KSLF_Buffer:
			pcView = NewMalloc<CBufferView>();
			((CBufferView*)pcView)->Init(0, 0, eRotation, szName.Text(), szFamily, false, 1, 1);
			break;
		case KSLF_Inverter:
			pcView = NewMalloc<CInverterView>();
			((CInverterView*)pcView)->Init(0, 0, eRotation, szName.Text(), szFamily, false, 1, 1);
			break;
		default:
			pcView = NewMalloc<CDTypeFlipFlopView>();
			((CDTypeFlipFlopView*)pcView)->Init(0, 0, eRotation, szName.Text(), szFamily, false);
			break;
		}
		szName.Kill();

		uiItem = AddItem(pcView, &sCentre, NULL);
		if (eFunction == KSLF_DFlipFlop)
		{
			for (i = 0; i < apcPins.NumElements(); i++)
			{
				if (aiGroups.GetValue(i) == (int)uiGroup)
				{
					szPortName = GetKicadSchemFlipFlopPortName(aszLetters.Get(i), aiNegated.GetValue(i) != 0);
					AddPinTerminal(pcSymbol, apcPins.GetPtr(i), pcView->GetPortView(szPortName), uiItem);
				}
			}
		}
		else
		{
			//The inputs' port views come first and then the output's.
			for (j = 0; j < aiInputs.NumElements(); j++)
			{
				AddPinTerminal(pcSymbol, apcPins.GetPtr(aiInputs.GetValue(j)), pcView->GetPortView(j), uiItem);
			}
			AddPinTerminal(pcSymbol, apcPins.GetPtr(iOutput), pcView->GetPortView(aiInputs.NumElements()), uiItem);
		}
		aiInputs.Kill();
		muiGates++;
	}

	asPositions.Kill();
	aiNegated.Kill();
	aszLetters.Kill();
	aiGroups.Kill();
	apcPins.Kill();
	aszIndices.Kill();
	return bValid;
}


//////////////////////////////////////////////////////////////////////////
//
// An instance of the placeholder subcircuit for the symbol's unit, turned
// as KiCad turns the symbol (a mirrored symbol isn't mirrored).
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemLogicimConverter::AddPlaceholder(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol)
{
	CCircuitView*							pcPlaceholder;
	CArrayTemplatePtr<CKicadSchemLibPin>	apcPins;
	CArrayChars								aszNames;
	CSubcircuitInstanceView*				pcInstance;
	SSubcircuitPinView*						psPin;
	size									uiItem;
	size									i;
	char*									szValue;

	pcPlaceholder = GetPlaceholderCircuit(pcSymbol, pcLibSymbol);
	if (pcPlaceholder == NULL)
	{
		return false;
	}

	szValue = pcSymbol->GetValue();
	pcInstance = NewMalloc<CSubcircuitInstanceView>();
	pcInstance->Init(0, 0, GetKicadRotation(ROT_East, pcSymbol->GetAngle()), pcSymbol->GetReference(), (szValue != NULL) ? szValue : "", pcPlaceholder, mpcFonts);
	uiItem = AddItem(pcInstance, pcSymbol->GetPosition(), NULL);

	apcPins.Init();
	aszNames.Init();
	GetPlaceholderPins(pcSymbol, pcLibSymbol, &apcPins, &aszNames);
	for (i = 0; i < apcPins.NumElements(); i++)
	{
		psPin = pcInstance->GetPin(aszNames.GetText(i));
		if (psPin != NULL)
		{
			AddPinTerminal(pcSymbol, apcPins.GetPtr(i), psPin->pcPortView, uiItem);
		}
	}
	aszNames.Kill();
	apcPins.Kill();
	muiPlaceholders++;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The placeholder for the library symbol's unit, made the first time.
// Its pins are on the sides the library symbol's are, in the same order
// along them.  It's named as the library symbol, without its library, and
// the unit's letter if it has more than one.
//
//////////////////////////////////////////////////////////////////////////
CCircuitView* CKicadSchemLogicimConverter::GetPlaceholderCircuit(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol)
{
	CChars									szTypeName;
	char*									szLibName;
	char*									szColon;
	CCircuitView*							pcPlaceholder;
	CArrayTemplatePtr<CKicadSchemLibPin>	apcPins;
	CArrayChars								aszNames;
	size									i;
	size									j;
	CKicadSchemLibPin*						pcPin;
	CKicadSchemLibPin*						pcOther;
	int										iAngle;
	int										iOtherAngle;
	EPinAlignment							eAlignment;
	int32									iWeight;
	CPinView*								pcPinView;
	SInt32Vec2								sBoxMinimum;
	SInt32Vec2								sBoxMaximum;
	bool									bBefore;

	szLibName = pcLibSymbol->GetName();
	szColon = strrchr(szLibName, ':');
	szTypeName.Init((szColon != NULL) ? szColon + 1 : szLibName);
	if (pcLibSymbol->NumUnits() > 1)
	{
		szTypeName.Append('_');
		szTypeName.Append((char)('A' + pcSymbol->GetUnit() - 1));
	}

	pcPlaceholder = mpcCircuits->Get(szTypeName.Text());
	if (pcPlaceholder != NULL)
	{
		szTypeName.Kill();
		return pcPlaceholder;
	}
	pcPlaceholder = mpcCircuits->Add(szTypeName.Text());
	szTypeName.Kill();
	if (pcPlaceholder == NULL)
	{
		return NULL;
	}

	apcPins.Init();
	aszNames.Init();
	GetPlaceholderPins(pcSymbol, pcLibSymbol, &apcPins, &aszNames);
	sBoxMinimum.Init(-2, -1);
	sBoxMaximum.Init(0, 1);
	for (i = 0; i < apcPins.NumElements(); i++)
	{
		//A pin points into the body, so a pin pointing right (0) is on the left.
		pcPin = apcPins.GetPtr(i);
		iAngle = ((int)floor(pcPin->GetAngle() / 90.0 + 0.5) % 4 + 4) % 4;
		switch (iAngle)
		{
		case 1:		eAlignment = PA_Bottom;	break;
		case 2:		eAlignment = PA_Right;	break;
		case 3:		eAlignment = PA_Top;	break;
		default:	eAlignment = PA_Left;	break;
		}

		//Down the left and right sides (library Y is up) and right to left along the top and bottom.
		iWeight = 0;
		for (j = 0; j < apcPins.NumElements(); j++)
		{
			pcOther = apcPins.GetPtr(j);
			iOtherAngle = ((int)floor(pcOther->GetAngle() / 90.0 + 0.5) % 4 + 4) % 4;
			if ((j == i) || (iOtherAngle != iAngle))
			{
				continue;
			}
			if ((iAngle == 0) || (iAngle == 2))
			{
				bBefore = (pcOther->GetPosition()->fY > pcPin->GetPosition()->fY) || ((pcOther->GetPosition()->fY == pcPin->GetPosition()->fY) && (j < i));
			}
			else
			{
				bBefore = (pcOther->GetPosition()->fX > pcPin->GetPosition()->fX) || ((pcOther->GetPosition()->fX == pcPin->GetPosition()->fX) && (j < i));
			}
			if (bBefore)
			{
				iWeight++;
			}
		}

		pcPinView = NewMalloc<CPinView>();
		pcPinView->Init(0, (int32)(3 * i), ROT_East, aszNames.GetText(i), "LVC", false, 1, RADIX_Hexadecimal, sBoxMinimum, sBoxMaximum);
		pcPinView->SetSubcircuitAppearance(eAlignment, PAN_Negative, iWeight, strstr(pcPin->GetGraphicStyle(), "inverted") != NULL, strstr(pcPin->GetGraphicStyle(), "clock") != NULL);
		pcPlaceholder->Add(pcPinView);
	}
	aszNames.Kill();
	apcPins.Kill();
	return pcPlaceholder;
}


//////////////////////////////////////////////////////////////////////////
//
// The unit's shown pins and the names their placeholder pins have: the
// pin's name, or its number if it has none, made unique by its number.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::GetPlaceholderPins(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol, CArrayTemplatePtr<CKicadSchemLibPin>* papcPins, CArrayChars* paszNames)
{
	size				i;
	CKicadSchemLibPin*	pcPin;
	CChars				szName;
	char*				szPinName;

	for (i = 0; i < pcLibSymbol->NumPins(); i++)
	{
		pcPin = pcLibSymbol->GetPin(i);
		if (!pcPin->IsOn(pcSymbol->GetUnit(), pcSymbol->GetBodyStyle()) || pcPin->IsHidden())
		{
			continue;
		}
		szPinName = pcPin->GetName();
		if (StrEmpty(szPinName) || (StringCompare(szPinName, "~") == 0))
		{
			szName.Init(pcPin->GetNumber());
		}
		else
		{
			szName.Init(szPinName);
		}
		if (paszNames->Contains(szName.Text()))
		{
			szName.Append('_');
			szName.Append(pcPin->GetNumber());
		}
		papcPins->Add(pcPin);
		paszNames->Add(szName.Text());
		szName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A tunnel named as its net at each label of each subnet of a net that
// has more than one.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::AddTunnels(void)
{
	size				i;
	size				j;
	size				k;
	CKicadSchemNet*		pcNet;
	CKicadSchemSubnet*	pcSubnet;
	CKicadSchemLabel*	pcLabel;
	CTunnelView*		pcTunnel;
	size				uiItem;

	for (i = 0; i < mpcNetlist->NumNets(); i++)
	{
		pcNet = mpcNetlist->GetNet(i);
		if (pcNet->NumSubnets() < 2)
		{
			continue;
		}
		for (j = 0; j < pcNet->NumSubnets(); j++)
		{
			pcSubnet = pcNet->GetSubnet(j);
			for (k = 0; k < pcSubnet->NumLabels(); k++)
			{
				pcLabel = pcSubnet->GetLabel(k);
				pcTunnel = NewMalloc<CTunnelView>();
				pcTunnel->Init(0, 0, GetKicadRotation(ROT_East, pcLabel->GetAngle()), pcNet->GetName(), false, mpcFonts);
				uiItem = AddItem(pcTunnel, pcLabel->GetPosition(), pcTunnel->GetPortView((size)0));
				AddTerminal(pcSubnet, pcLabel->GetPosition(), pcTunnel->GetPortView((size)0), uiItem);
				muiTunnels++;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::AddConnections(void)
{
	size	i;

	for (i = 0; i < mpcNetlist->NumSubnets(); i++)
	{
		AddSubnetConnections(mpcNetlist->GetSubnet(i), i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The subnet's terminals as a tree: each next is the terminal fewest wire
// pieces from those already in it, connected to the one it reaches.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::AddSubnetConnections(CKicadSchemSubnet* pcSubnet, size uiSubnet)
{
	CArrayInt						aiTerminals;
	CArrayInt						aiNodes;
	CArrayInt						aiConnected;
	CArrayInt						aiTargets;
	CArrayInt						aiPath;
	CArrayInt						aiBestPath;
	CArrayKicadSchemPoint			asPath;
	CKicadSchemSubnetGraph			cGraph;
	size							i;
	size							j;
	int								iBest;
	int								iTo;
	SKicadSchemLogicimConnection*	psConnection;
	bool							bHorizontal;

	aiTerminals.Init();
	for (i = 0; i < masTerminals.NumElements(); i++)
	{
		if (masTerminals.Get(i)->pcSubnet == pcSubnet)
		{
			aiTerminals.Add((int)i);
		}
	}
	if (aiTerminals.NumElements() < 2)
	{
		aiTerminals.Kill();
		return;
	}

	cGraph.Init(pcSubnet);
	aiNodes.Init();
	for (i = 0; i < aiTerminals.NumElements(); i++)
	{
		aiNodes.Add((int)cGraph.AddPoint(&masTerminals.Get(aiTerminals.GetValue(i))->sKicadPosition));
	}
	cGraph.Build();

	aiConnected.Init();
	aiConnected.Add(0);
	aiTargets.Init();
	aiBestPath.Init();
	while (aiConnected.NumElements() < aiTerminals.NumElements())
	{
		aiTargets.ReInit();
		for (i = 0; i < aiConnected.NumElements(); i++)
		{
			aiTargets.Add(aiNodes.GetValue(aiConnected.GetValue(i)));
		}

		iBest = -1;
		aiBestPath.ReInit();
		for (i = 0; i < aiTerminals.NumElements(); i++)
		{
			if (aiConnected.Find((int)i) != ARRAY_ELEMENT_NOT_FOUND)
			{
				continue;
			}
			aiPath.Init();
			if (cGraph.FindPath(aiNodes.GetValue(i), &aiTargets, &aiPath))
			{
				if ((iBest == -1) || (aiPath.NumElements() < aiBestPath.NumElements()))
				{
					iBest = (int)i;
					aiBestPath.ReInit();
					for (j = 0; j < aiPath.NumElements(); j++)
					{
						aiBestPath.Add(aiPath.GetValue(j));
					}
				}
			}
			aiPath.Kill();
		}

		psConnection = masConnections.Add();
		psConnection->uiSubnet = uiSubnet;
		if (iBest == -1)
		{
			//Not joined by wires (only by being at the same place): straight to the first.
			for (i = 0; i < aiTerminals.NumElements(); i++)
			{
				if (aiConnected.Find((int)i) == ARRAY_ELEMENT_NOT_FOUND)
				{
					break;
				}
			}
			iBest = (int)i;
			iTo = 0;
			psConnection->fBend = 0.5;
		}
		else
		{
			iTo = 0;
			for (i = 0; i < aiConnected.NumElements(); i++)
			{
				if (aiNodes.GetValue(aiConnected.GetValue(i)) == aiBestPath.GetValue(aiBestPath.NumElements() - 1))
				{
					iTo = aiConnected.GetValue(i);
					break;
				}
			}
			asPath.Init();
			for (i = 0; i < aiBestPath.NumElements(); i++)
			{
				asPath.Add(cGraph.GetPoint(aiBestPath.GetValue(i)));
			}
			psConnection->fBend = CKicadSchemSubnetGraph::GetBendFraction(&asPath, &bHorizontal);
			asPath.Kill();
		}
		psConnection->bConnected = false;
		psConnection->uiFrom = aiTerminals.GetValue(iBest);
		psConnection->uiTo = aiTerminals.GetValue(iTo);
		aiConnected.Add(iBest);
	}

	aiBestPath.Kill();
	aiTargets.Kill();
	aiConnected.Kill();
	aiNodes.Kill();
	cGraph.Kill();
	aiTerminals.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the view, made at (0, 0), to the circuit.  The anchor is the port
// that goes where KiCad has the thing, if not the view's position.
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemLogicimConverter::AddItem(CComponentView* pcView, SKicadSchemPoint* psKicadPosition, CPortView* pcAnchor)
{
	SKicadSchemLogicimItem*		psItem;

	mpcCircuit->Add(pcView);
	psItem = masItems.Add();
	psItem->pcView = pcView;
	psItem->sKicadPosition.Init(psKicadPosition->fX / KICAD_SCHEM_LOGICIM_GRID, psKicadPosition->fY / KICAD_SCHEM_LOGICIM_GRID);
	psItem->pcPort = pcAnchor;
	psItem->pcAttachedTo = NULL;
	if (pcAnchor != NULL)
	{
		psItem->sAnchor = pcAnchor->GetGridPosition();
	}
	else
	{
		psItem->sAnchor.Init(0, 0);
	}
	return masItems.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::AddTerminal(CKicadSchemSubnet* pcSubnet, SKicadSchemPoint* psKicadPosition, CPortView* pcPortView, size uiItem)
{
	SKicadSchemLogicimTerminal*		psTerminal;

	if ((pcSubnet == NULL) || (pcPortView == NULL))
	{
		return;
	}
	psTerminal = masTerminals.Add();
	psTerminal->pcSubnet = pcSubnet;
	psTerminal->sKicadPosition = *psKicadPosition;
	psTerminal->pcPortView = pcPortView;
	psTerminal->uiItem = uiItem;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::AddPinTerminal(CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin, CPortView* pcPortView, size uiItem)
{
	SKicadSchemPoint	sPosition;

	mpcFile->GetPinPosition(&sPosition, pcSymbol, pcPin);
	AddTerminal(mpcNetlist->GetPinSubnet(pcSymbol, pcPin), &sPosition, pcPortView, uiItem);
}


//////////////////////////////////////////////////////////////////////////
//
// Each item at its KiCad place spread out from the origin, and then moved
// apart where they overlap.  Attached items go with their pins.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::Place(float64 fSpread)
{
	size						i;
	SKicadSchemLogicimItem*		psItem;

	for (i = 0; i < masItems.NumElements(); i++)
	{
		psItem = masItems.Get(i);
		if (psItem->pcAttachedTo == NULL)
		{
			psItem->pcView->SetPosition((int32)floor(psItem->sKicadPosition.x * fSpread + 0.5) - psItem->sAnchor.x, (int32)floor(psItem->sKicadPosition.y * fSpread + 0.5) - psItem->sAnchor.y);
		}
	}
	PlaceAttachedItems();
	SeparateItems();
}


//////////////////////////////////////////////////////////////////////////
//
// Each attached item with its port on the port it's attached to.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::PlaceAttachedItems(void)
{
	size						i;
	SKicadSchemLogicimItem*		psItem;
	SInt32Vec2					sPort;
	SInt32Vec2					sTarget;
	SInt32Vec2					sPosition;

	for (i = 0; i < masItems.NumElements(); i++)
	{
		psItem = masItems.Get(i);
		if (psItem->pcAttachedTo != NULL)
		{
			sPort = psItem->pcPort->GetGridPosition();
			sTarget = psItem->pcAttachedTo->GetGridPosition();
			sPosition = psItem->pcView->GetPosition();
			psItem->pcView->SetPosition(sPosition.x + sTarget.x - sPort.x, sPosition.y + sTarget.y - sPort.y);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A tunnel or supply that KiCad has on a pin (of a gate, flip flop or
// placeholder) is attached to its port, turned so its port faces the
// pin's.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::AttachItems(void)
{
	size							i;
	size							j;
	size							k;
	SKicadSchemLogicimItem*			psItem;
	SKicadSchemLogicimTerminal*		psTerminal;
	SKicadSchemLogicimTerminal*		psOther;
	CPortView*						pcTarget;
	int								iWanted;
	ERotation						aeRotations[4] = { ROT_East, ROT_South, ROT_West, ROT_North };
	ERotation						eOriginal;

	for (i = 0; i < masItems.NumElements(); i++)
	{
		psItem = masItems.Get(i);
		if (psItem->pcPort == NULL)
		{
			continue;
		}

		pcTarget = NULL;
		for (j = 0; (j < masTerminals.NumElements()) && (pcTarget == NULL); j++)
		{
			psTerminal = masTerminals.Get(j);
			if (psTerminal->uiItem != i)
			{
				continue;
			}
			for (k = 0; k < masTerminals.NumElements(); k++)
			{
				psOther = masTerminals.Get(k);
				if ((psOther->pcSubnet == psTerminal->pcSubnet) && (masItems.Get(psOther->uiItem)->pcPort == NULL) && psOther->sKicadPosition.Equals(&psTerminal->sKicadPosition))
				{
					pcTarget = psOther->pcPortView;
					break;
				}
			}
		}
		if (pcTarget == NULL)
		{
			continue;
		}

		psItem->pcAttachedTo = pcTarget;
		iWanted = (CTraceAutoConnector::GetPortFacing(pcTarget) + 2) % 4;
		eOriginal = psItem->pcView->GetRotation();
		for (k = 0; k < 4; k++)
		{
			psItem->pcView->SetRotation(aeRotations[k]);
			if (CTraceAutoConnector::GetPortFacing(psItem->pcPort) == iWanted)
			{
				break;
			}
		}
		if (k == 4)
		{
			psItem->pcView->SetRotation(eOriginal);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The item's box and the boxes of the items attached to it.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemLogicimConverter::GetItemBox(size uiItem, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum)
{
	size						i;
	SKicadSchemLogicimItem*		psItem;
	SKicadSchemLogicimItem*		psOther;
	SFloat64Vec2				sMinimum;
	SFloat64Vec2				sMaximum;

	psItem = masItems.Get(uiItem);
	if (!psItem->pcView->GetBoundingBox(psMinimum, psMaximum))
	{
		return false;
	}
	for (i = 0; i < masItems.NumElements(); i++)
	{
		psOther = masItems.Get(i);
		if ((psOther->pcAttachedTo != NULL) && (psOther->pcAttachedTo->GetOwner() == psItem->pcView) && psOther->pcView->GetBoundingBox(&sMinimum, &sMaximum))
		{
			IncludeBoundsPoint(psMinimum, psMaximum, sMinimum.x, sMinimum.y);
			IncludeBoundsPoint(psMinimum, psMaximum, sMaximum.x, sMaximum.y);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the box, spaced, overlaps any of the boxes placed so far.
//
//////////////////////////////////////////////////////////////////////////
static bool IsKicadSchemLogicimBoxFree(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, int32 iMoveX, int32 iMoveY, CArrayTemplate<SFloat64Vec2>* pasMinimums, CArrayTemplate<SFloat64Vec2>* pasMaximums, size uiPlaced)
{
	size			i;
	SFloat64Vec2*	psOtherMinimum;
	SFloat64Vec2*	psOtherMaximum;

	for (i = 0; i < uiPlaced; i++)
	{
		psOtherMinimum = pasMinimums->Get(i);
		psOtherMaximum = pasMaximums->Get(i);
		if ((psMinimum->x + iMoveX < psOtherMaximum->x + KICAD_SCHEM_LOGICIM_SPACING) && (psOtherMinimum->x < psMaximum->x + iMoveX + KICAD_SCHEM_LOGICIM_SPACING) &&
			(psMinimum->y + iMoveY < psOtherMaximum->y + KICAD_SCHEM_LOGICIM_SPACING) && (psOtherMinimum->y < psMaximum->y + iMoveY + KICAD_SCHEM_LOGICIM_SPACING))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Places the items one at a time, each where it is if that's clear of the
// ones already placed, otherwise at the nearest place that is (searching
// out in rings).  An item's box takes in the items attached to it.  Items
// that couldn't be placed clear are counted as overlaps.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::SeparateItems(void)
{
	CArrayTemplate<SFloat64Vec2>	asMinimums;
	CArrayTemplate<SFloat64Vec2>	asMaximums;
	size							i;
	size							uiPlaced;
	SKicadSchemLogicimItem*			psItem;
	SFloat64Vec2					sMinimum;
	SFloat64Vec2					sMaximum;
	int32							iRadius;
	int32							iStep;
	int32							iMoveX;
	int32							iMoveY;
	int32							iBestX;
	int32							iBestY;
	int32							iBestDistance;
	int32							iDistance;
	bool							bFound;
	SInt32Vec2						sPosition;

	PlaceAttachedItems();
	asMinimums.Init();
	asMaximums.Init();
	uiPlaced = 0;
	muiOverlaps = 0;
	for (i = 0; i < masItems.NumElements(); i++)
	{
		psItem = masItems.Get(i);
		if ((psItem->pcAttachedTo != NULL) || !GetItemBox(i, &sMinimum, &sMaximum))
		{
			continue;
		}

		iBestX = 0;
		iBestY = 0;
		bFound = IsKicadSchemLogicimBoxFree(&sMinimum, &sMaximum, 0, 0, &asMinimums, &asMaximums, uiPlaced);
		for (iRadius = 1; !bFound && (iRadius <= KICAD_SCHEM_LOGICIM_SEPARATE_RADIUS); iRadius++)
		{
			//The ring's places nearest first.
			iBestDistance = -1;
			for (iStep = -iRadius; iStep <= iRadius; iStep++)
			{
				for (iMoveY = -iRadius; iMoveY <= iRadius; iMoveY += 2 * iRadius)
				{
					iMoveX = iStep;
					iDistance = abs(iMoveX) + abs(iMoveY);
					if (((iBestDistance == -1) || (iDistance < iBestDistance)) && IsKicadSchemLogicimBoxFree(&sMinimum, &sMaximum, iMoveX, iMoveY, &asMinimums, &asMaximums, uiPlaced))
					{
						iBestDistance = iDistance;
						iBestX = iMoveX;
						iBestY = iMoveY;
					}
				}
				for (iMoveX = -iRadius; iMoveX <= iRadius; iMoveX += 2 * iRadius)
				{
					iMoveY = iStep;
					iDistance = abs(iMoveX) + abs(iMoveY);
					if (((iBestDistance == -1) || (iDistance < iBestDistance)) && IsKicadSchemLogicimBoxFree(&sMinimum, &sMaximum, iMoveX, iMoveY, &asMinimums, &asMaximums, uiPlaced))
					{
						iBestDistance = iDistance;
						iBestX = iMoveX;
						iBestY = iMoveY;
					}
				}
			}
			bFound = iBestDistance != -1;
		}
		if (!bFound)
		{
			muiOverlaps++;
			iBestX = 0;
			iBestY = 0;
		}

		sPosition = psItem->pcView->GetPosition();
		psItem->pcView->SetPosition(sPosition.x + iBestX, sPosition.y + iBestY);
		sMinimum.x += iBestX;
		sMaximum.x += iBestX;
		sMinimum.y += iBestY;
		sMaximum.y += iBestY;
		asMinimums.Add(&sMinimum);
		asMaximums.Add(&sMaximum);
		uiPlaced++;
	}
	PlaceAttachedItems();
	asMaximums.Kill();
	asMinimums.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Rewires the circuit from nothing and returns how many connections
// couldn't be routed.  Each subnet is a net, and connections between the
// same two items are a run.
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemLogicimConverter::Route(void)
{
	CTraceAutoConnector				cConnector;
	CArrayInt						aiSubnetNets;
	CArrayInt						aiRunFrom;
	CArrayInt						aiRunTo;
	CArrayInt						aiRunCounts;
	CArrayInt						aiRuns;
	CArrayInt						aiConnectorIndices;
	size							i;
	size							j;
	SKicadSchemLogicimConnection*	psConnection;
	SKicadSchemLogicimTerminal*		psFrom;
	SKicadSchemLogicimTerminal*		psTo;
	int								iNet;
	int								iRun;
	int								iFromItem;
	int								iToItem;
	size							uiUnconnected;

	mpcCircuit->KillTraces();

	//The connections between each pair of items.
	aiRunFrom.Init();
	aiRunTo.Init();
	aiRunCounts.Init();
	for (i = 0; i < masConnections.NumElements(); i++)
	{
		psConnection = masConnections.Get(i);
		iFromItem = (int)masTerminals.Get(psConnection->uiFrom)->uiItem;
		iToItem = (int)masTerminals.Get(psConnection->uiTo)->uiItem;
		for (j = 0; j < aiRunFrom.NumElements(); j++)
		{
			if ((aiRunFrom.GetValue(j) == iFromItem) && (aiRunTo.GetValue(j) == iToItem))
			{
				break;
			}
		}
		if (j == aiRunFrom.NumElements())
		{
			aiRunFrom.Add(iFromItem);
			aiRunTo.Add(iToItem);
			aiRunCounts.Add(0);
		}
		aiRunCounts.SetValue(j, aiRunCounts.GetValue(j) + 1);
	}

	cConnector.Init(mpcCircuit);
	aiRuns.Init();
	for (i = 0; i < aiRunCounts.NumElements(); i++)
	{
		aiRuns.Add((aiRunCounts.GetValue(i) > 1) ? (int)cConnector.AddRun() : -1);
	}
	aiSubnetNets.Init();
	for (i = 0; i < mpcNetlist->NumSubnets(); i++)
	{
		aiSubnetNets.Add(-1);
	}

	aiConnectorIndices.Init();
	for (i = 0; i < masConnections.NumElements(); i++)
	{
		psConnection = masConnections.Get(i);
		iNet = aiSubnetNets.GetValue(psConnection->uiSubnet);
		if (iNet == -1)
		{
			iNet = (int)cConnector.AddNet();
			aiSubnetNets.SetValue(psConnection->uiSubnet, iNet);
		}
		psFrom = masTerminals.Get(psConnection->uiFrom);
		psTo = masTerminals.Get(psConnection->uiTo);
		iRun = -1;
		for (j = 0; j < aiRunFrom.NumElements(); j++)
		{
			if ((aiRunFrom.GetValue(j) == (int)psFrom->uiItem) && (aiRunTo.GetValue(j) == (int)psTo->uiItem))
			{
				iRun = aiRuns.GetValue(j);
				break;
			}
		}
		aiConnectorIndices.Add(cConnector.AddConnection(iNet, psFrom->pcPortView, psTo->pcPortView, psConnection->fBend, iRun) ? (int)(cConnector.NumConnections() - 1) : -1);
	}
	cConnector.Connect();
	uiUnconnected = 0;
	for (i = 0; i < masConnections.NumElements(); i++)
	{
		psConnection = masConnections.Get(i);
		psConnection->bConnected = (aiConnectorIndices.GetValue(i) != -1) && cConnector.IsConnected(aiConnectorIndices.GetValue(i));
		if (!psConnection->bConnected)
		{
			uiUnconnected++;
		}
	}
	aiConnectorIndices.Kill();
	cConnector.Kill();

	aiSubnetNets.Kill();
	aiRuns.Kill();
	aiRunCounts.Kill();
	aiRunTo.Kill();
	aiRunFrom.Kill();
	return uiUnconnected;
}


//////////////////////////////////////////////////////////////////////////
//
// The rotation of a thing that is eUpright when KiCad hasn't turned it,
// turned anticlockwise by KiCad's angle (to the nearest right angle).
//
//////////////////////////////////////////////////////////////////////////
ERotation CKicadSchemLogicimConverter::GetKicadRotation(ERotation eUpright, float64 fAngle)
{
	int		iQuarters;

	iQuarters = (((int)floor(fAngle / 90.0 + 0.5)) % 4 + 4) % 4;
	return RotateRight(eUpright, (4 - iQuarters) % 4);
}


//////////////////////////////////////////////////////////////////////////
//
// Passive parts and mechanical symbols, by their reference's letters or
// their library symbol.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemLogicimConverter::IsSkippedSymbol(CKicadSchemSymbol* pcSymbol)
{
	CChars	szPrefix;
	char*	szReference;
	size	i;
	bool	bSkipped;

	for (i = 0; i < sizeof(gaszKicadSchemSkippedLibraryNames) / sizeof(const char*); i++)
	{
		if ((strstr(pcSymbol->GetLibId(), gaszKicadSchemSkippedLibraryNames[i]) != NULL) || (strstr(pcSymbol->GetLibSymbolName(), gaszKicadSchemSkippedLibraryNames[i]) != NULL))
		{
			return true;
		}
	}

	szReference = pcSymbol->GetReference();
	if (szReference == NULL)
	{
		return false;
	}
	for (i = 0; ((szReference[i] >= 'A') && (szReference[i] <= 'Z')) || ((szReference[i] >= 'a') && (szReference[i] <= 'z')); i++)
	{
	}
	szPrefix.Init(szReference, 0, i);
	szPrefix.UpperCase();
	bSkipped = false;
	for (i = 0; i < sizeof(gaszKicadSchemSkippedReferences) / sizeof(const char*); i++)
	{
		if (szPrefix.Equals(gaszKicadSchemSkippedReferences[i]))
		{
			bSkipped = true;
			break;
		}
	}
	szPrefix.Kill();
	return bSkipped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemLogicimConverter::IsGroundName(const char* szName)
{
	CChars	szUpper;
	bool	bGround;

	szUpper.Init(szName);
	szUpper.UpperCase();
	bGround = szUpper.StartsWith("GND") || szUpper.StartsWith("VSS") || szUpper.StartsWith("AGND") || szUpper.StartsWith("DGND") || szUpper.StartsWith("EARTH") || szUpper.Equals("0V");
	szUpper.Kill();
	return bGround;
}


//////////////////////////////////////////////////////////////////////////
//
// The voltage in a supply's name: "+5V" is 5, "3.3V" and "+3V3" are 3.3
// and "12VOut" is 12.  A name without one (VCC) is 5.
//
//////////////////////////////////////////////////////////////////////////
float32 CKicadSchemLogicimConverter::GetSupplyVoltage(const char* szName)
{
	size		i;
	float64		fVoltage;
	float64		fScale;

	for (i = 0; (szName[i] != '\0') && ((szName[i] < '0') || (szName[i] > '9')); i++)
	{
	}
	if (szName[i] == '\0')
	{
		return 5.0f;
	}

	fVoltage = 0;
	for (; (szName[i] >= '0') && (szName[i] <= '9'); i++)
	{
		fVoltage = fVoltage * 10 + (szName[i] - '0');
	}
	if ((szName[i] == '.') || (((szName[i] == 'V') || (szName[i] == 'v')) && (szName[i + 1] >= '0') && (szName[i + 1] <= '9')))
	{
		i++;
		fScale = 0.1;
		for (; (szName[i] >= '0') && (szName[i] <= '9'); i++)
		{
			fVoltage += fScale * (szName[i] - '0');
			fScale /= 10;
		}
	}
	if (fVoltage <= 0)
	{
		return 5.0f;
	}
	return (float32)fVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuitView* CKicadSchemLogicimConverter::GetCircuit(void) { return mpcCircuit; }
float64 CKicadSchemLogicimConverter::GetSpread(void) { return mfSpread; }
size CKicadSchemLogicimConverter::NumConnections(void) { return masConnections.NumElements(); }
size CKicadSchemLogicimConverter::NumUnconnected(void) { return muiUnconnected; }
SKicadSchemLogicimConnection* CKicadSchemLogicimConverter::GetConnection(size uiIndex) { return masConnections.SafeGet(uiIndex); }
size CKicadSchemLogicimConverter::NumTerminals(void) { return masTerminals.NumElements(); }
SKicadSchemLogicimTerminal* CKicadSchemLogicimConverter::GetTerminal(size uiIndex) { return masTerminals.SafeGet(uiIndex); }
size CKicadSchemLogicimConverter::NumItems(void) { return masItems.NumElements(); }
size CKicadSchemLogicimConverter::NumGates(void) { return muiGates; }
size CKicadSchemLogicimConverter::NumPlaceholders(void) { return muiPlaceholders; }
size CKicadSchemLogicimConverter::NumPowers(void) { return muiPowers; }
size CKicadSchemLogicimConverter::NumTunnels(void) { return muiTunnels; }
size CKicadSchemLogicimConverter::NumSkipped(void) { return muiSkipped; }
size CKicadSchemLogicimConverter::NumOverlaps(void) { return muiOverlaps; }


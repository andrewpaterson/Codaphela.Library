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
#include "BaseLib/PointerFunctions.h"
#include "LogicimLib/AndGateView.h"
#include "LogicimLib/OrGateView.h"
#include "LogicimLib/BufferView.h"
#include "LogicimLib/Logic74View.h"
#include "LogicimLib/FlipFlopView.h"
#include "LogicimLib/PowerSourceView.h"
#include "LogicimLib/TunnelView.h"
#include "LogicimLib/SplitterView.h"
#include "LogicimLib/ValueView.h"
#include "LogicimLib/SubcircuitInstanceView.h"
#include "LogicimLib/TraceAutoConnector.h"
#include "LogicimLib/ComponentShapes.h"
#include "KicadSchemLogicimConverter.h"


//The spreads the components are placed at, from KiCad's places, widest first.
static float64 gafKicadSchemLogicimSpreads[] = { 3.0, 2.5, 2.1, 1.8, 1.55, 1.35, 1.2, 1.1, 1.0 };


//The rotations, turning right from East, to try something in each of.
static ERotation gaeKicadSchemLogicimRotations[4] = { ROT_East, ROT_South, ROT_West, ROT_North };


//Reference prefixes and library names of the symbols that are left out.
static const char* gaszKicadSchemSkippedReferences[] = { "R", "RN", "C", "CN", "L", "FB", "D", "LED", "H", "MH", "TP", "NT", "FID" };
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
	muiSplitters = 0;
	muiBlocks = 0;
	muiSkipped = 0;
	muiOverlaps = 0;
	muiExplosions = 0;
	muiExcessBefore = 0;
	muiExcessAfter = 0;
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
	size	j;
	float64	fWorse;
	float64	fSpread;

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
	AddBuses();
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
	if (i < uiNumSpreads)
	{
		//Halfway between the last spread that wired and the first that didn't, twice.
		fWorse = gafKicadSchemLogicimSpreads[i];
		for (j = 0; j < 2; j++)
		{
			fSpread = (mfSpread + fWorse) / 2;
			Place(fSpread);
			if (Route() > uiFirstUnconnected)
			{
				fWorse = fSpread;
			}
			else
			{
				mfSpread = fSpread;
			}
		}
	}
	Place(mfSpread);
	muiUnconnected = Route();
	Explode();
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
	CChars						szPartType;
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

	//A real 74 series part that isn't gates.
	szFamily.Init();
	szPartType.Init();
	if (ParseKicadSchemLogic74Part(pcLibSymbol->GetName(), &szFamily, &szPartType) || ParseKicadSchemLogic74Part(pcSymbol->GetValue(), &szFamily, &szPartType))
	{
		if (AddLogic74Block(pcSymbol, pcLibSymbol, szFamily.Text(), szPartType.Text()))
		{
			szPartType.Kill();
			szFamily.Kill();
			return true;
		}
	}
	szPartType.Kill();
	szFamily.Kill();

	szFamily.Init();
	szPartType.Init();
	eFunction = ParseKicadSchemLogicFunction(pcLibSymbol->GetName(), &szFamily, &szPartType);
	if (eFunction == KSLF_None)
	{
		eFunction = ParseKicadSchemLogicFunction(pcSymbol->GetValue(), &szFamily, &szPartType);
	}
	bAdded = false;
	if (eFunction != KSLF_None)
	{
		bAdded = AddLogicParts(pcSymbol, pcLibSymbol, eFunction, szFamily.Text(), szPartType.Text());
	}
	szPartType.Kill();
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


//////////////////////////////////////////////////////////////////////////
//
// A gate (or buffer) of one bit inputs at the origin, with implied power.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
static CComponentView* NewKicadSchemGateView(ERotation eRotation, const char* szName, const char* szFamily, int iInputCount)
{
	M*	pcGate;

	pcGate = NewMalloc<M>();
	pcGate->Init(0, 0, eRotation, szName, szFamily, false, iInputCount, 1);
	return pcGate;
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
bool CKicadSchemLogicimConverter::AddLogicParts(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol, EKicadSchemLogicFunction eFunction, const char* szFamily, const char* szPartType)
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
	bool								bPackaged;

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

	//A real 74 series gate part is its package, or the unit of it KiCad draws.
	bPackaged = false;
	if (bValid && (eFunction != KSLF_DFlipFlop))
	{
		bPackaged = AddLogic74Part(pcSymbol, pcLibSymbol, eFunction, &aszIndices, &apcPins, &aiGroups, &aszLetters, &asPositions, szFamily, szPartType);
	}

	for (uiGroup = 0; bValid && !bPackaged && (uiGroup < aszIndices.NumElements()); uiGroup++)
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
			pcView = NewKicadSchemGateView<CAndGateView>(eRotation, szName.Text(), szFamily, iInputs);
			break;
		case KSLF_Nand:
			pcView = NewKicadSchemGateView<CNandGateView>(eRotation, szName.Text(), szFamily, iInputs);
			break;
		case KSLF_Or:
			pcView = NewKicadSchemGateView<COrGateView>(eRotation, szName.Text(), szFamily, iInputs);
			break;
		case KSLF_Nor:
			pcView = NewKicadSchemGateView<CNorGateView>(eRotation, szName.Text(), szFamily, iInputs);
			break;
		case KSLF_Xor:
			pcView = NewKicadSchemGateView<CXorGateView>(eRotation, szName.Text(), szFamily, iInputs);
			break;
		case KSLF_Xnor:
			pcView = NewKicadSchemGateView<CXnorGateView>(eRotation, szName.Text(), szFamily, iInputs);
			break;
		case KSLF_Buffer:
			pcView = NewKicadSchemGateView<CBufferView>(eRotation, szName.Text(), szFamily, 1);
			break;
		case KSLF_Inverter:
			pcView = NewKicadSchemGateView<CInverterView>(eRotation, szName.Text(), szFamily, 1);
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
// The generic gate a KiCad logic function is.
//
//////////////////////////////////////////////////////////////////////////
static bool GetKicadSchemGateFunction(EKicadSchemLogicFunction eFunction, EGateFunction* peGateFunction)
{
	switch (eFunction)
	{
	case KSLF_And:			*peGateFunction = GF_And;		return true;
	case KSLF_Nand:			*peGateFunction = GF_Nand;		return true;
	case KSLF_Or:			*peGateFunction = GF_Or;		return true;
	case KSLF_Nor:			*peGateFunction = GF_Nor;		return true;
	case KSLF_Xor:			*peGateFunction = GF_Xor;		return true;
	case KSLF_Xnor:			*peGateFunction = GF_Xnor;		return true;
	case KSLF_Buffer:		*peGateFunction = GF_Buffer;	return true;
	case KSLF_Inverter:		*peGateFunction = GF_Inverter;	return true;
	default:				return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The real 74 series part (see SLogic74Part) the symbol's unit is of, as
// KiCad draws it.  A unit of a part drawn as units ("74LVC00_Multi") is a
// unit view of the part, named as the part; a single gate part ("1G08")
// is a unit view too.  A part drawn whole is a package, each pin on the
// port of its name ("2B").  False (and nothing added) if there's no row
// for the part or its pins aren't its row's gates.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemLogicimConverter::AddLogic74Part(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol, EKicadSchemLogicFunction eFunction, CArrayChars* paszIndices, CArrayTemplatePtr<CKicadSchemLibPin>* papcPins, CArrayInt* paiGroups, CArrayChars* paszLetters, CArrayKicadSchemPoint* pasPositions, const char* szFamily, const char* szPartType)
{
	SLogic74PartType		sType;
	EGateFunction			eGateFunction;
	size					i;
	size					j;
	int						iInputs;
	int						iIndex;
	int						iUnit;
	SKicadSchemPoint		sCentre;
	SKicadSchemPoint		sInputs;
	SKicadSchemPoint		sOutputs;
	int						iOutputs;
	CArrayInt				aiInputs;
	int						iOutput;
	CLogic74PackageView*	pcPackage;
	CLogic74UnitView*		pcUnit;
	CComponentView*			pcView;
	size					uiItem;
	CChars					szPortName;
	CPortView*				pcPortView;
	bool					bUnit;

	if (!FindLogic74Part(&sType, szPartType) || !GetKicadSchemGateFunction(eFunction, &eGateFunction))
	{
		return false;
	}
	if ((sType.psPart->eKind != L74K_Gates) || (sType.psPart->eGateFunction != eGateFunction))
	{
		return false;
	}

	//Every gate has the row's inputs, and a gate number the part has.
	for (i = 0; i < paszIndices->NumElements(); i++)
	{
		iInputs = 0;
		for (j = 0; j < papcPins->NumElements(); j++)
		{
			if ((paiGroups->GetValue(j) == (int)i) && !paszLetters->Get(j)->Equals("Y"))
			{
				iInputs++;
			}
		}
		if (iInputs != sType.psPart->iInputCount)
		{
			return false;
		}
		iIndex = paszIndices->Get(i)->Empty() ? 1 : atoi(paszIndices->GetText(i));
		if ((iIndex < 1) || (iIndex > sType.iUnitCount))
		{
			return false;
		}
	}

	bUnit = (pcLibSymbol->NumUnits() > 1) || (sType.iUnitCount == 1);
	if (bUnit && (paszIndices->NumElements() != 1))
	{
		return false;
	}

	sCentre.Init();
	sInputs.Init();
	sOutputs.Init();
	iOutputs = 0;
	iOutput = -1;
	aiInputs.Init();
	for (i = 0; i < papcPins->NumElements(); i++)
	{
		sCentre.fX += pasPositions->Get(i)->fX;
		sCentre.fY += pasPositions->Get(i)->fY;
		if (paszLetters->Get(i)->Equals("Y"))
		{
			sOutputs.fX += pasPositions->Get(i)->fX;
			sOutputs.fY += pasPositions->Get(i)->fY;
			iOutputs++;
			iOutput = (int)i;
		}
		else
		{
			sInputs.fX += pasPositions->Get(i)->fX;
			sInputs.fY += pasPositions->Get(i)->fY;
			for (j = 0; j < aiInputs.NumElements(); j++)
			{
				if (strcmp(paszLetters->GetText(i), paszLetters->GetText(aiInputs.GetValue(j))) < 0)
				{
					break;
				}
			}
			aiInputs.InsertAt((int)i, j);
		}
	}
	sCentre.fX /= (float64)papcPins->NumElements();
	sCentre.fY /= (float64)papcPins->NumElements();
	sInputs.fX /= (float64)aiInputs.NumElements();
	sInputs.fY /= (float64)aiInputs.NumElements();
	sOutputs.fX /= (float64)iOutputs;
	sOutputs.fY /= (float64)iOutputs;

	if (bUnit)
	{
		//The unit's inputs in letter order, then its output, as the generic gate's.
		iUnit = (pcLibSymbol->NumUnits() > 1) ? pcSymbol->GetUnit() - 1 : 0;
		pcUnit = NewMalloc<CLogic74UnitView>();
		pcUnit->Init(0, 0, GetKicadSchemFacing(&sInputs, &sOutputs), pcSymbol->GetReference(), szFamily, false, szPartType, iUnit);
		pcView = pcUnit;
		uiItem = AddItem(pcView, &sCentre, NULL);
		for (j = 0; j < aiInputs.NumElements(); j++)
		{
			szPortName.Init();
			CPins::CalculatePortName(&szPortName, "Input", j, aiInputs.NumElements());
			AddPinTerminal(pcSymbol, papcPins->GetPtr(aiInputs.GetValue(j)), pcView->GetPortView(szPortName.Text()), uiItem);
			szPortName.Kill();
		}
		AddPinTerminal(pcSymbol, papcPins->GetPtr(iOutput), pcView->GetPortView("Output"), uiItem);
		muiGates++;
	}
	else
	{
		pcPackage = NewMalloc<CLogic74PackageView>();
		pcPackage->Init(0, 0, GetKicadSchemFacing(&sInputs, &sOutputs), pcSymbol->GetReference(), szFamily, false, szPartType);
		pcView = pcPackage;
		uiItem = AddItem(pcView, &sCentre, NULL);
		for (i = 0; i < papcPins->NumElements(); i++)
		{
			szPortName.Init();
			szPortName.Append(paszIndices->Get(paiGroups->GetValue(i))->Empty() ? "1" : paszIndices->GetText(paiGroups->GetValue(i)));
			szPortName.Append(paszLetters->Get(i));
			pcPortView = pcView->GetPortView(szPortName.Text());
			szPortName.Kill();
			if (pcPortView != NULL)
			{
				AddPinTerminal(pcSymbol, papcPins->GetPtr(i), pcPortView, uiItem);
			}
		}
		muiGates += paszIndices->NumElements();
	}
	aiInputs.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A KiCad pin's name as the label of the port it's on (see
// CLogicBlockView): its overbars ("~{OE}", "1~{OE}") taken out, and
// pbNegated whether it had any; a sigma ("Σ1", a sum) is an S.
//
//////////////////////////////////////////////////////////////////////////
static void GetKicadSchemPinLabel(const char* szName, CChars* pszLabel, bool* pbNegated)
{
	size	i;

	*pbNegated = false;
	for (i = 0; (szName != NULL) && (szName[i] != '\0'); i++)
	{
		if ((szName[i] == '~') && (szName[i + 1] == '{'))
		{
			*pbNegated = true;
			i++;
		}
		else if (szName[i] == '}')
		{
		}
		else if (((unsigned char)szName[i] == 0xCE) && ((unsigned char)szName[i + 1] == 0xA3))
		{
			pszLabel->Append('S');
			i++;
		}
		else
		{
			pszLabel->Append(szName[i]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The other names a pin's label goes by: a 74F283's carries ("C0", "C4")
// are a 74LVC283's "CIN" and "COUT", and a bus switch's channel letter
// after the name ("I0A", "YA", "EA") is the channel's number before it
// ("1I0", "1Y", "1E").  False if there's no other name.
//
//////////////////////////////////////////////////////////////////////////
static bool GetKicadSchemPinLabelAlias(const char* szLabel, CChars* pszAlias)
{
	size	uiLength;
	char	cChannel;
	CChars	szName;

	if (StringCompare(szLabel, "C0") == 0)
	{
		pszAlias->Set("CIN");
		return true;
	}
	if (StringCompare(szLabel, "C4") == 0)
	{
		pszAlias->Set("COUT");
		return true;
	}
	uiLength = StrLen(szLabel);
	if ((uiLength >= 2) && ((szLabel[0] == 'I') || (szLabel[0] == 'Y') || (szLabel[0] == 'E')))
	{
		cChannel = szLabel[uiLength - 1];
		if ((cChannel >= 'A') && (cChannel <= 'D'))
		{
			szName.Init(szLabel, 0, uiLength - 1);
			pszAlias->Kill();
			pszAlias->Init();
			pszAlias->Append((int32)(cChannel - 'A' + 1));
			pszAlias->Append(szName.Text());
			szName.Kill();
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The view's port labelled szLabel and inverted or not, or the only one
// labelled szLabel.  A unit's labels have no flip flop number ("1CP" is
// "CP").
//
//////////////////////////////////////////////////////////////////////////
static CPortView* FindKicadSchemLabelledPort(CComponentView* pcView, const char* szLabel, bool bNegated, bool bUnit)
{
	size		i;
	CPortView*	pcPortView;
	CPortView*	pcOnly;
	size		uiLabelled;

	if (bUnit)
	{
		while ((*szLabel >= '0') && (*szLabel <= '9'))
		{
			szLabel++;
		}
	}
	pcOnly = NULL;
	uiLabelled = 0;
	for (i = 0; i < pcView->NumPortViews(); i++)
	{
		pcPortView = pcView->GetPortView(i);
		if (StringCompare(pcPortView->GetText(), szLabel) == 0)
		{
			if (pcPortView->IsInverting() == bNegated)
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
// The view's port for each of the unit's logic pins (not its power or
// unconnected pins), in the order of the pins.  False if a pin has none.
//
//////////////////////////////////////////////////////////////////////////
static bool MapKicadSchemLogic74Pins(CComponentView* pcView, CArrayTemplatePtr<CKicadSchemLibPin>* papcPins, CArrayPortViewPtr* papcPorts, bool bUnit)
{
	size		i;
	CChars		szLabel;
	CChars		szAlias;
	bool		bNegated;
	CPortView*	pcPortView;

	papcPorts->Kill();
	papcPorts->Init();
	for (i = 0; i < papcPins->NumElements(); i++)
	{
		szLabel.Init();
		GetKicadSchemPinLabel(papcPins->GetPtr(i)->GetName(), &szLabel, &bNegated);
		pcPortView = FindKicadSchemLabelledPort(pcView, szLabel.Text(), bNegated, bUnit);
		if (pcPortView == NULL)
		{
			szAlias.Init();
			if (GetKicadSchemPinLabelAlias(szLabel.Text(), &szAlias))
			{
				pcPortView = FindKicadSchemLabelledPort(pcView, szAlias.Text(), bNegated, bUnit);
			}
			szAlias.Kill();
		}
		szLabel.Kill();
		if (pcPortView == NULL)
		{
			return false;
		}
		papcPorts->Add(pcPortView);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A package (or unit) view of the part turned eRotation.
//
//////////////////////////////////////////////////////////////////////////
static CComponentView* NewKicadSchemLogic74View(CKicadSchemSymbol* pcSymbol, ERotation eRotation, const char* szFamily, const char* szPartType, bool bUnit, int iUnit)
{
	CLogic74PackageView*	pcPackage;
	CLogic74UnitView*		pcUnit;

	if (bUnit)
	{
		pcUnit = NewMalloc<CLogic74UnitView>();
		pcUnit->Init(0, 0, eRotation, pcSymbol->GetReference(), szFamily, false, szPartType, iUnit);
		return pcUnit;
	}
	pcPackage = NewMalloc<CLogic74PackageView>();
	pcPackage->Init(0, 0, eRotation, pcSymbol->GetReference(), szFamily, false, szPartType);
	return pcPackage;
}


//////////////////////////////////////////////////////////////////////////
//
// A real 74 series part that isn't gates (see SLogic74Part): a package of
// the whole part, or a unit of a part KiCad draws as units (a 74LVC74's
// flip flop).  Each KiCad pin is on the port its name labels (see
// GetKicadSchemPinLabel).  The view is turned so the pins KiCad has on
// the left of the part are on its left.  False (and nothing added) if a
// logic pin has no port.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemLogicimConverter::AddLogic74Block(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol, const char* szFamily, const char* szPartType)
{
	SLogic74PartType						sType;
	CArrayTemplatePtr<CKicadSchemLibPin>	apcPins;
	CArrayPortViewPtr						apcPorts;
	CArrayKicadSchemPoint					asPositions;
	CKicadSchemLibPin*						pcPin;
	SKicadSchemPoint						sPosition;
	SKicadSchemPoint						sCentre;
	SKicadSchemPoint						sLeft;
	SKicadSchemPoint						sRight;
	int										iLeft;
	int										iRight;
	bool									bUnit;
	int										iUnit;
	size									i;
	CComponentView*							pcView;
	bool									bMapped;
	size									uiItem;
	ERotation								eRotation;

	if (!FindLogic74Part(&sType, szPartType) || (sType.psPart->eKind != L74K_Block))
	{
		return false;
	}
	bUnit = (pcLibSymbol->NumUnits() > 1) && (sType.iUnitCount > 1);
	iUnit = bUnit ? pcSymbol->GetUnit() - 1 : 0;

	apcPins.Init();
	asPositions.Init();
	for (i = 0; i < pcLibSymbol->NumPins(); i++)
	{
		pcPin = pcLibSymbol->GetPin(i);
		if (!pcPin->IsOn(pcSymbol->GetUnit(), pcSymbol->GetBodyStyle()) || (pcPin->GetType() == KSPT_PowerIn) || (pcPin->GetType() == KSPT_NoConnect) || IsKicadSchemPowerPinName(pcPin->GetName()))
		{
			continue;
		}
		apcPins.Add(pcPin);
		mpcFile->GetPinPosition(&sPosition, pcSymbol, pcPin);
		asPositions.Add(&sPosition);
	}
	if (apcPins.NumElements() == 0)
	{
		asPositions.Kill();
		apcPins.Kill();
		return false;
	}

	//Which side of the part's middle each pin's port is on, unturned, gives the way KiCad turned it.
	apcPorts.Init();
	pcView = NewKicadSchemLogic74View(pcSymbol, ROT_East, szFamily, szPartType, bUnit, iUnit);
	bMapped = MapKicadSchemLogic74Pins(pcView, &apcPins, &apcPorts, bUnit);
	sCentre.Init();
	sLeft.Init();
	sRight.Init();
	iLeft = 0;
	iRight = 0;
	for (i = 0; bMapped && (i < apcPins.NumElements()); i++)
	{
		sCentre.fX += asPositions.Get(i)->fX;
		sCentre.fY += asPositions.Get(i)->fY;
		if (apcPorts.GetPtr(i)->GetGridPosition().x < 0)
		{
			sLeft.fX += asPositions.Get(i)->fX;
			sLeft.fY += asPositions.Get(i)->fY;
			iLeft++;
		}
		else
		{
			sRight.fX += asPositions.Get(i)->fX;
			sRight.fY += asPositions.Get(i)->fY;
			iRight++;
		}
	}
	pcView->Kill();
	if (!bMapped)
	{
		apcPorts.Kill();
		asPositions.Kill();
		apcPins.Kill();
		return false;
	}

	sCentre.fX /= (float64)apcPins.NumElements();
	sCentre.fY /= (float64)apcPins.NumElements();
	eRotation = ROT_East;
	if ((iLeft > 0) && (iRight > 0))
	{
		sLeft.fX /= (float64)iLeft;
		sLeft.fY /= (float64)iLeft;
		sRight.fX /= (float64)iRight;
		sRight.fY /= (float64)iRight;
		eRotation = GetKicadSchemFacing(&sLeft, &sRight);
	}

	pcView = NewKicadSchemLogic74View(pcSymbol, eRotation, szFamily, szPartType, bUnit, iUnit);
	MapKicadSchemLogic74Pins(pcView, &apcPins, &apcPorts, bUnit);
	uiItem = AddItem(pcView, &sCentre, NULL);
	for (i = 0; i < apcPins.NumElements(); i++)
	{
		AddPinTerminal(pcSymbol, apcPins.GetPtr(i), apcPorts.GetPtr(i), uiItem);
	}
	muiBlocks++;

	apcPorts.Kill();
	asPositions.Kill();
	apcPins.Kill();
	return true;
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
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::AddBuses(void)
{
	size	i;

	for (i = 0; i < mpcNetlist->NumBuses(); i++)
	{
		AddBus(mpcNetlist->GetBus(i), i);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The way from a tap's bus point to its wire point, across the bus (a
// TRACE_DIRECTION): East or West for a bus going up and down, South or
// North for one going across.
//
//////////////////////////////////////////////////////////////////////////
static int GetKicadSchemTapSide(SKicadSchemBusTap* psTap)
{
	if (psTap->bVertical)
	{
		return (psTap->sWirePoint.fX >= psTap->sBusPoint.fX) ? TRACE_DIRECTION_EAST : TRACE_DIRECTION_WEST;
	}
	return (psTap->sWirePoint.fY >= psTap->sBusPoint.fY) ? TRACE_DIRECTION_SOUTH : TRACE_DIRECTION_NORTH;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float64 GetKicadSchemTapAlong(SKicadSchemBusTap* psTap)
{
	return psTap->bVertical ? psTap->sBusPoint.fY : psTap->sBusPoint.fX;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the bus is across the way it runs at the tap.
//
//////////////////////////////////////////////////////////////////////////
static float64 GetKicadSchemTapAcross(SKicadSchemBusTap* psTap)
{
	return psTap->bVertical ? psTap->sBusPoint.fX : psTap->sBusPoint.fY;
}


//////////////////////////////////////////////////////////////////////////
//
// A splitter for each run of the bus's taps on one side of it, and a tunnel at each of
// its labels if its name is on another bus too.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::AddBus(CKicadSchemBus* pcBus, size uiBus)
{
	CArrayInt				aiTaps;
	CArrayInt				aiUsed;
	CArrayInt				aiRun;
	size					i;
	size					j;
	size					uiMember;
	size					uiNamed;
	SKicadSchemBusTap*		psTap;
	SKicadSchemBusTap*		psOther;
	CKicadSchemLabel*		pcLabel;
	CTunnelView*			pcTunnel;
	size					uiItem;
	bool					bBefore;

	if (pcBus->NumMembers() == 0)
	{
		return;
	}

	uiNamed = 0;
	if (pcBus->GetName() != NULL)
	{
		for (i = 0; i < mpcNetlist->NumBuses(); i++)
		{
			if ((mpcNetlist->GetBus(i)->GetName() != NULL) && (StringCompare(mpcNetlist->GetBus(i)->GetName(), pcBus->GetName()) == 0))
			{
				uiNamed++;
			}
		}
	}
	if (uiNamed > 1)
	{
		for (i = 0; i < pcBus->NumLabels(); i++)
		{
			pcLabel = pcBus->GetLabel(i);
			pcTunnel = NewMalloc<CTunnelView>();
			pcTunnel->Init(0, 0, GetKicadRotation(ROT_East, pcLabel->GetAngle()), pcBus->GetName(), false, mpcFonts);
			uiItem = AddItem(pcTunnel, pcLabel->GetPosition(), pcTunnel->GetPortView((size)0));
			AddTerminal(pcBus->GetSubnet(), pcLabel->GetPosition(), pcTunnel->GetPortView((size)0), uiItem, true);
			muiTunnels++;
		}
	}

	//The taps, sorted by side, then where the bus is across, then along it.
	aiTaps.Init();
	aiUsed.Init();
	for (i = 0; i < pcBus->NumTaps(); i++)
	{
		psTap = pcBus->GetTap(i);
		uiMember = pcBus->GetMemberIndex(psTap->pcMember->GetNet()->GetName());
		if (uiMember == ARRAY_ELEMENT_NOT_FOUND)
		{
			continue;
		}
		for (j = 0; j < aiTaps.NumElements(); j++)
		{
			psOther = pcBus->GetTap(aiTaps.GetValue(j));
			if (psTap->bVertical != psOther->bVertical)
			{
				bBefore = psTap->bVertical;
			}
			else if (GetKicadSchemTapSide(psTap) != GetKicadSchemTapSide(psOther))
			{
				bBefore = GetKicadSchemTapSide(psTap) < GetKicadSchemTapSide(psOther);
			}
			else if (fabs(GetKicadSchemTapAcross(psTap) - GetKicadSchemTapAcross(psOther)) > KICAD_SCHEM_POINT_TOLERANCE)
			{
				bBefore = GetKicadSchemTapAcross(psTap) < GetKicadSchemTapAcross(psOther);
			}
			else
			{
				bBefore = GetKicadSchemTapAlong(psTap) < GetKicadSchemTapAlong(psOther);
			}
			if (bBefore)
			{
				break;
			}
		}
		aiTaps.InsertAt((int)i, j);
	}

	//Runs: the same side of the same piece of the bus, each tap no more than two entries' spacing from the last, and no member twice (a splitter
	//has one end for each bit).
	aiRun.Init();
	for (i = 0; i < aiTaps.NumElements(); i++)
	{
		psTap = pcBus->GetTap(aiTaps.GetValue(i));
		if (aiRun.NumElements() > 0)
		{
			psOther = pcBus->GetTap(aiRun.GetValue(aiRun.NumElements() - 1));
			if ((psTap->bVertical != psOther->bVertical) || (GetKicadSchemTapSide(psTap) != GetKicadSchemTapSide(psOther)) ||
				(fabs(GetKicadSchemTapAcross(psTap) - GetKicadSchemTapAcross(psOther)) > KICAD_SCHEM_POINT_TOLERANCE) || (GetKicadSchemTapAlong(psTap) - GetKicadSchemTapAlong(psOther) > 2 * KICAD_SCHEM_LOGICIM_GRID + KICAD_SCHEM_POINT_TOLERANCE) ||
				(aiUsed.Find((int)pcBus->GetMemberIndex(psTap->pcMember->GetNet()->GetName())) != ARRAY_ELEMENT_NOT_FOUND))
			{
				AddBusSplitter(pcBus, &aiRun, psOther->bVertical, GetKicadSchemTapSide(psOther));
				aiRun.ReInit();
				aiUsed.ReInit();
			}
		}
		aiRun.Add(aiTaps.GetValue(i));
		aiUsed.Add((int)pcBus->GetMemberIndex(psTap->pcMember->GetNet()->GetName()));
	}
	if (aiRun.NumElements() > 0)
	{
		psOther = pcBus->GetTap(aiRun.GetValue(aiRun.NumElements() - 1));
		AddBusSplitter(pcBus, &aiRun, psOther->bVertical, GetKicadSchemTapSide(psOther));
	}
	aiRun.Kill();
	aiUsed.Kill();
	aiTaps.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A splitter of the whole bus for a run of taps (in order along the bus)
// on the side iSide of it.  It's turned so its ends face the members, and
// its ends are given to the taps in the order they're along the bus.  The
// bus's other members go to an end of their own, after the taps'.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::AddBusSplitter(CKicadSchemBus* pcBus, CArrayInt* paiTaps, bool bVertical, int iSide)
{
	ERotation				eRotation;
	CSplitterView*			pcSplitter;
	CArrayInt				aiFans;
	CArrayInt				aiAlongs;
	int*					aiSplitIndices;
	int32					iBitWidth;
	int32					iFanOut;
	int32					iFan;
	int32					iRestFan;
	int32					iAlong;
	size					i;
	size					j;
	size					uiTaps;
	size					uiItem;
	SKicadSchemBusTap*		psTap;
	SKicadSchemPoint		sCentre;
	CPortView*				pcEnd;

	uiTaps = paiTaps->NumElements();
	iBitWidth = (int32)pcBus->NumMembers();
	iFanOut = (int32)uiTaps + (((size)iBitWidth > uiTaps) ? 1 : 0);

	//The rotation that has the ends facing the members, and where each end is along the bus.
	eRotation = ROT_East;
	aiAlongs.Init();
	for (i = 0; i < 4; i++)
	{
		pcSplitter = NewMalloc<CSplitterView>();
		pcSplitter->Init(0, 0, gaeKicadSchemLogicimRotations[i], "", iBitWidth, iFanOut, 1, SA_LeftHanded, 0);
		pcEnd = pcSplitter->GetEndPortView(0);
		if ((pcEnd != NULL) && (CTraceAutoConnector::GetPortFacing(pcEnd) == iSide))
		{
			eRotation = gaeKicadSchemLogicimRotations[i];
			for (iFan = 0; iFan < iFanOut; iFan++)
			{
				pcEnd = pcSplitter->GetEndPortView(iFan);
				iAlong = 0;
				if (pcEnd != NULL)
				{
					iAlong = bVertical ? pcEnd->GetGridPosition().y : pcEnd->GetGridPosition().x;
				}
				aiAlongs.Add(iAlong);
			}
			SafeKill(pcSplitter);
			break;
		}
		SafeKill(pcSplitter);
	}
	if (aiAlongs.NumElements() == 0)
	{
		for (iFan = 0; iFan < iFanOut; iFan++)
		{
			aiAlongs.Add(iFan);
		}
	}

	//The ends in order along the bus.
	aiFans.Init();
	for (iFan = 0; iFan < iFanOut; iFan++)
	{
		for (j = 0; j < aiFans.NumElements(); j++)
		{
			if (aiAlongs.GetValue(iFan) < aiAlongs.GetValue(aiFans.GetValue(j)))
			{
				break;
			}
		}
		aiFans.InsertAt(iFan, j);
	}

	aiSplitIndices = (int*)malloc(sizeof(int) * (iBitWidth + 1));
	iRestFan = ((size)iFanOut > uiTaps) ? aiFans.GetValue(uiTaps) : 0;
	for (i = 0; i < (size)iBitWidth; i++)
	{
		aiSplitIndices[i] = iRestFan;
	}
	sCentre.Init();
	for (i = 0; i < uiTaps; i++)
	{
		psTap = pcBus->GetTap(paiTaps->GetValue(i));
		aiSplitIndices[pcBus->GetMemberIndex(psTap->pcMember->GetNet()->GetName())] = aiFans.GetValue(i);
		sCentre.fX += psTap->sBusPoint.fX;
		sCentre.fY += psTap->sBusPoint.fY;
	}
	sCentre.fX /= (float64)uiTaps;
	sCentre.fY /= (float64)uiTaps;

	pcSplitter = NewMalloc<CSplitterView>();
	pcSplitter->Init(0, 0, eRotation, "", iBitWidth, iFanOut, 1, SA_LeftHanded, 0, aiSplitIndices);
	free(aiSplitIndices);
	uiItem = AddItem(pcSplitter, &sCentre, NULL);

	psTap = pcBus->GetTap(paiTaps->GetValue(0));
	AddTerminal(pcBus->GetSubnet(), &psTap->sBusPoint, pcSplitter->GetStartPortView(), uiItem, true);
	for (i = 0; i < uiTaps; i++)
	{
		psTap = pcBus->GetTap(paiTaps->GetValue(i));
		AddTerminal(psTap->pcMember, &psTap->sWirePoint, pcSplitter->GetEndPortView(aiFans.GetValue(i)), uiItem);
	}
	aiFans.Kill();
	aiAlongs.Kill();
	muiSplitters++;
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
				if (mpcNetlist->IsBusLabel(pcLabel))
				{
					continue;
				}
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
	for (i = 0; i < mpcNetlist->NumBuses(); i++)
	{
		AddSubnetConnections(mpcNetlist->GetBus(i)->GetSubnet(), mpcNetlist->NumSubnets() + i);
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
	psItem->sOffset.Init(0, 0);
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
void CKicadSchemLogicimConverter::AddTerminal(CKicadSchemSubnet* pcSubnet, SKicadSchemPoint* psKicadPosition, CPortView* pcPortView, size uiItem, bool bBus)
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
	psTerminal->bBus = bBus;
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
// Explodes the dense areas while that wires as well and leaves fewer
// knots over the dense areas' limit, as many times as
// KICAD_SCHEM_LOGICIM_EXPLODE_PASSES.  If exploding them all is worse,
// exploding just the densest is tried before giving up.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::Explode(void)
{
	CArrayTemplate<SFloat64Vec2>	asCentres;
	CArrayTemplate<SInt32Vec2>		asOffsets;
	size							uiPass;
	size							i;
	size							uiExcess;
	size							uiNewExcess;
	size							uiUnconnected;
	bool							bOnlyDensest;

	asCentres.Init();
	asOffsets.Init();
	uiExcess = FindDenseAreas(&asCentres);
	muiExcessBefore = uiExcess;
	bOnlyDensest = false;
	for (uiPass = 0; (uiPass < KICAD_SCHEM_LOGICIM_EXPLODE_PASSES) && (asCentres.NumElements() > 0); uiPass++)
	{
		asOffsets.ReInit();
		for (i = 0; i < masItems.NumElements(); i++)
		{
			asOffsets.Add(&masItems.Get(i)->sOffset);
		}

		if (bOnlyDensest)
		{
			while (asCentres.NumElements() > 1)
			{
				asCentres.RemoveTail();
			}
		}
		ExplodeAreas(&asCentres);
		Place(mfSpread);
		uiUnconnected = Route();
		asCentres.ReInit();
		uiNewExcess = FindDenseAreas(&asCentres);
		if ((uiUnconnected > muiUnconnected) || (uiNewExcess >= uiExcess))
		{
			//Worse: back to how it was.
			for (i = 0; i < masItems.NumElements(); i++)
			{
				masItems.Get(i)->sOffset = *asOffsets.Get(i);
			}
			Place(mfSpread);
			muiUnconnected = Route();
			if (bOnlyDensest)
			{
				break;
			}
			bOnlyDensest = true;
			asCentres.ReInit();
			FindDenseAreas(&asCentres);
			continue;
		}
		muiUnconnected = uiUnconnected;
		uiExcess = uiNewExcess;
		muiExplosions++;
		bOnlyDensest = false;
	}
	muiExcessAfter = uiExcess;
	asOffsets.Kill();
	asCentres.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Makes room through each dense area by putting space in along a line
// across it each way: everything right of its centre moves right and
// everything below it moves down, by KICAD_SCHEM_LOGICIM_EXPLODE_SPACE.
// Things on the same side of a line move together, so what was lined up
// stays lined up and a wire across a line only gets longer.  Attached
// items move with their pins.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemLogicimConverter::ExplodeAreas(CArrayTemplate<SFloat64Vec2>* pasCentres)
{
	size						i;
	size						j;
	SKicadSchemLogicimItem*		psItem;
	SInt32Vec2					sPosition;
	SFloat64Vec2*				psCentre;

	for (i = 0; i < masItems.NumElements(); i++)
	{
		psItem = masItems.Get(i);
		if (psItem->pcAttachedTo != NULL)
		{
			continue;
		}
		sPosition = psItem->pcView->GetPosition();
		sPosition.x += psItem->sAnchor.x;
		sPosition.y += psItem->sAnchor.y;
		for (j = 0; j < pasCentres->NumElements(); j++)
		{
			psCentre = pasCentres->Get(j);
			if (sPosition.x > psCentre->x)
			{
				psItem->sOffset.x += KICAD_SCHEM_LOGICIM_EXPLODE_SPACE;
			}
			if (sPosition.y > psCentre->y)
			{
				psItem->sOffset.y += KICAD_SCHEM_LOGICIM_EXPLODE_SPACE;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The dense windows (with more knots than KICAD_SCHEM_LOGICIM_DENSE_KNOTS),
// densest first and none overlapping another: the centres of the first
// KICAD_SCHEM_LOGICIM_DENSE_AREAS, and the knots over the limit in all of
// them (up to KICAD_SCHEM_LOGICIM_DENSE_SCAN).  A knot is
// where a wire turns or ends (but not on a port) or where two wires cross.
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemLogicimConverter::FindDenseAreas(CArrayTemplate<SFloat64Vec2>* pasCentres)
{
	CArrayTemplate<SInt32Vec2>	asKnots;
	CArrayPortViewPtr			apcPorts;
	CArrayInt					aiSums;
	CArrayInt					aiUsed;
	size						i;
	size						j;
	CTraceView*					pcTrace;
	CTraceView*					pcOther;
	SInt32Vec2					sPoint;
	SInt32Vec2					sMinimum;
	SInt32Vec2					sMaximum;
	int32						iWidth;
	int32						iHeight;
	int32						x;
	int32						y;
	int32						iWindow;
	int32						iCount;
	int32						iBest;
	int32						iBestX;
	int32						iBestY;
	size						uiDensest;
	size						uiArea;
	bool						bPort;
	SFloat64Vec2				sCentre;

	apcPorts.Init();
	mpcCircuit->CollectPortViews(&apcPorts);
	asKnots.Init();
	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		pcTrace = mpcCircuit->GetTrace(i);
		for (j = 0; j < 2; j++)
		{
			sPoint = (j == 0) ? pcTrace->GetStart() : pcTrace->GetEnd();
			bPort = false;
			for (x = 0; x < (int32)apcPorts.NumElements(); x++)
			{
				if (apcPorts.GetPtr(x)->GetGridPosition() == sPoint)
				{
					bPort = true;
					break;
				}
			}
			if (!bPort)
			{
				asKnots.Add(&sPoint);
			}
		}
		for (j = i + 1; j < mpcCircuit->NumTraces(); j++)
		{
			pcOther = mpcCircuit->GetTrace(j);
			if ((pcTrace->GetStart().y == pcTrace->GetEnd().y) && (pcOther->GetStart().x == pcOther->GetEnd().x))
			{
				sPoint.Init(pcOther->GetStart().x, pcTrace->GetStart().y);
			}
			else if ((pcTrace->GetStart().x == pcTrace->GetEnd().x) && (pcOther->GetStart().y == pcOther->GetEnd().y))
			{
				sPoint.Init(pcTrace->GetStart().x, pcOther->GetStart().y);
			}
			else
			{
				continue;
			}
			if (pcTrace->Contains(sPoint) && pcOther->Contains(sPoint) && !pcTrace->IsEnd(sPoint) && !pcOther->IsEnd(sPoint))
			{
				asKnots.Add(&sPoint);
			}
		}
	}
	apcPorts.Kill();
	if (asKnots.NumElements() == 0)
	{
		asKnots.Kill();
		return 0;
	}

	//Knots counted into cells, summed so any window's count is four sums.
	sMinimum = *asKnots.Get(0);
	sMaximum = sMinimum;
	for (i = 1; i < asKnots.NumElements(); i++)
	{
		sPoint = *asKnots.Get(i);
		if (sPoint.x < sMinimum.x)
		{
			sMinimum.x = sPoint.x;
		}
		if (sPoint.y < sMinimum.y)
		{
			sMinimum.y = sPoint.y;
		}
		if (sPoint.x > sMaximum.x)
		{
			sMaximum.x = sPoint.x;
		}
		if (sPoint.y > sMaximum.y)
		{
			sMaximum.y = sPoint.y;
		}
	}
	iWindow = KICAD_SCHEM_LOGICIM_DENSE_WINDOW;
	iWidth = sMaximum.x - sMinimum.x + 1 + iWindow;
	iHeight = sMaximum.y - sMinimum.y + 1 + iWindow;
	uiArea = (size)(iWidth + 1) * (size)(iHeight + 1);
	aiSums.Init();
	aiSums.GrowToAtLeastNumElements(uiArea, true, 0);
	for (i = 0; i < asKnots.NumElements(); i++)
	{
		sPoint = *asKnots.Get(i);
		x = sPoint.x - sMinimum.x + 1;
		y = sPoint.y - sMinimum.y + 1;
		aiSums.SetValue(x + y * (iWidth + 1), aiSums.GetValue(x + y * (iWidth + 1)) + 1);
	}
	for (y = 1; y <= iHeight; y++)
	{
		for (x = 1; x <= iWidth; x++)
		{
			aiSums.SetValue(x + y * (iWidth + 1), aiSums.GetValue(x + y * (iWidth + 1)) + aiSums.GetValue(x - 1 + y * (iWidth + 1)) + aiSums.GetValue(x + (y - 1) * (iWidth + 1)) - aiSums.GetValue(x - 1 + (y - 1) * (iWidth + 1)));
		}
	}

	aiUsed.Init();
	aiUsed.GrowToAtLeastNumElements(uiArea, true, 0);
	uiDensest = 0;
	for (i = 0; i < KICAD_SCHEM_LOGICIM_DENSE_SCAN; i++)
	{
		iBest = -1;
		iBestX = 0;
		iBestY = 0;
		for (y = iWindow; y <= iHeight; y++)
		{
			for (x = iWindow; x <= iWidth; x++)
			{
				if (aiUsed.GetValue(x + y * (iWidth + 1)) != 0)
				{
					continue;
				}
				iCount = aiSums.GetValue(x + y * (iWidth + 1)) - aiSums.GetValue(x - iWindow + y * (iWidth + 1)) - aiSums.GetValue(x + (y - iWindow) * (iWidth + 1)) + aiSums.GetValue(x - iWindow + (y - iWindow) * (iWidth + 1));
				if (iCount > iBest)
				{
					iBest = iCount;
					iBestX = x;
					iBestY = y;
				}
			}
		}
		if (iBest <= KICAD_SCHEM_LOGICIM_DENSE_KNOTS)
		{
			break;
		}
		uiDensest += (size)(iBest - KICAD_SCHEM_LOGICIM_DENSE_KNOTS);

		//The window covers the cells from iBest - iWindow + 1 to iBest, less one for the sums' border.
		if (i < KICAD_SCHEM_LOGICIM_DENSE_AREAS)
		{
			sCentre.Init(sMinimum.x - 1 + iBestX - (iWindow - 1) / 2.0, sMinimum.y - 1 + iBestY - (iWindow - 1) / 2.0);
			pasCentres->Add(&sCentre);
		}
		for (y = iBestY - iWindow + 1; y < iBestY + iWindow; y++)
		{
			for (x = iBestX - iWindow + 1; x < iBestX + iWindow; x++)
			{
				if ((x >= 0) && (x <= iWidth) && (y >= 0) && (y <= iHeight))
				{
					aiUsed.SetValue(x + y * (iWidth + 1), 1);
				}
			}
		}
	}
	aiUsed.Kill();
	aiSums.Kill();
	asKnots.Kill();
	return uiDensest;
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
			psItem->pcView->SetPosition((int32)floor(psItem->sKicadPosition.x * fSpread + 0.5) - psItem->sAnchor.x + psItem->sOffset.x, (int32)floor(psItem->sKicadPosition.y * fSpread + 0.5) - psItem->sAnchor.y + psItem->sOffset.y);
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
		iWanted = CTraceAutoConnector::OppositeDirection(CTraceAutoConnector::GetPortFacing(pcTarget));
		eOriginal = psItem->pcView->GetRotation();
		for (k = 0; k < 4; k++)
		{
			psItem->pcView->SetRotation(gaeKicadSchemLogicimRotations[k]);
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
	for (i = 0; i < mpcNetlist->NumSubnets() + mpcNetlist->NumBuses(); i++)
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
size CKicadSchemLogicimConverter::NumSplitters(void) { return muiSplitters; }
size CKicadSchemLogicimConverter::NumBlocks(void) { return muiBlocks; }
size CKicadSchemLogicimConverter::NumSkipped(void) { return muiSkipped; }
size CKicadSchemLogicimConverter::NumOverlaps(void) { return muiOverlaps; }
size CKicadSchemLogicimConverter::NumExplosions(void) { return muiExplosions; }
size CKicadSchemLogicimConverter::GetExcessKnotsBefore(void) { return muiExcessBefore; }
size CKicadSchemLogicimConverter::GetExcessKnotsAfter(void) { return muiExcessAfter; }


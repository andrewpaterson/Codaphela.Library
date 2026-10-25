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
#include <math.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Logger.h"
#include "MicroprocessorLib/Constant.h"
#include "MicroprocessorLib/Probe.h"
#include "MicroprocessorLib/LogicPort.h"
#include "MicroprocessorLib/PowerPinNames.h"
#include "MicroprocessorLib/Pin.h"
#include "MicroprocessorLib/Trace.h"
#include "MicroprocessorLib/Voltage.h"
#include "MicroprocessorLib/FamilyVoltageConfigurationStore.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "BaseLib/StringHelper.h"
#include "ComponentStateXML.h"
#include "ValueView.h"


//////////////////////////////////////////////////////////////////////////
//
// Subclasses add their shapes, the port views, the value text and then
// CalculateBounds.
//
//////////////////////////////////////////////////////////////////////////
void CValueView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, SInt32Vec2 sBoxMinimum, SInt32Vec2 sBoxMaximum)
{
	CIntegratedCircuitView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);
	miBitWidth = iBitWidth;
	meRadix = eRadix;
	miMaxDigits = CalculateMaxDigits(eRadix, iBitWidth);
	msBoxMinimum = sBoxMinimum;
	msBoxMaximum = sBoxMaximum;
	muiValueText = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The box for the widest value, as Logicim's createGraphics: its length is
// the width of that many monospaced Xs and half a grid unit, rounded up.
//
//////////////////////////////////////////////////////////////////////////
void CValueView::MeasureBox(CCircuitFonts* pcFonts, int32 iBitWidth, ERadix eRadix, SInt32Vec2* psBoxMinimum, SInt32Vec2* psBoxMaximum)
{
	CChars				szWidest;
	SCircuitTextSize	sSize;
	int32				iLength;

	szWidest.Init();
	szWidest.Append('X', (size)CalculateMaxDigits(eRadix, iBitWidth));
	sSize = pcFonts->Measure(CF_Monospaced, VALUE_FONT_SIZE, szWidest.Text());
	szWidest.Kill();

	iLength = (int32)ceil(sSize.fWidth + 0.5);
	psBoxMinimum->Init(-iLength, -1);
	psBoxMaximum->Init(0, 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The bus at the position and, with explicit power, VCC and GND on the
// box's top and bottom a grid unit along.
//
//////////////////////////////////////////////////////////////////////////
void CValueView::CreateValuePortViews(const char* szPortName)
{
	CArrayChars		aszNames;
	CChars			szName;
	int32			i;

	aszNames.Init();
	for (i = 0; i < miBitWidth; i++)
	{
		szName.Init(szPortName);
		szName.Append(' ');
		szName.Append(i);
		aszNames.Add(szName.Text());
		szName.Kill();
	}
	AddPortView(&aszNames, 0, 0);
	aszNames.Kill();

	if (mbExplicitPowerPorts)
	{
		AddPortView(POWER_PIN_VCC, msBoxMaximum.x - 1, msBoxMinimum.y);
		AddPortView(POWER_PIN_GND, msBoxMaximum.x - 1, msBoxMaximum.y);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim draws these views turned twice to the right, so their text
// faces the other way to the component's rotation.
//
//////////////////////////////////////////////////////////////////////////
void CValueView::AddValueText(void)
{
	SFloat64Vec2	sCentre;

	sCentre = GetBoxCentre();
	muiValueText = mcShapes.NumShapes();
	mcShapes.AddText(sCentre.x, sCentre.y, "", CF_Monospaced, VALUE_FONT_SIZE, TA_Centre, 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2 CValueView::GetBoxCentre(void)
{
	SFloat64Vec2	sCentre;

	sCentre.Init((msBoxMinimum.x + msBoxMaximum.x) / 2.0, (msBoxMinimum.y + msBoxMaximum.y) / 2.0);
	return sCentre;
}


//////////////////////////////////////////////////////////////////////////
//
// The value on the bus in the circuit simulation of the path at its time,
// or nothing without a simulation or a component in it (as Logicim's
// RadixHelper.getStringValue).
//
//////////////////////////////////////////////////////////////////////////
void CValueView::GetValueText(CChars* psz, CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	CPortView*			pcBus;
	STraceValue			asValues[RADIX_MAX_BIT_WIDTH];
	int32				i;
	CPort*				pcPort;
	CCircuitComponent*	pcComponent;
	int64				iTime;

	if (pcCircuitSimulation == NULL)
	{
		return;
	}
	pcComponent = GetComponent(pcViewPath, pcCircuitSimulation);
	pcBus = GetPortView((size)0);
	if ((pcComponent == NULL) || (pcBus == NULL) || (pcBus->NumPorts(pcViewPath, pcCircuitSimulation) != (size)miBitWidth))
	{
		return;
	}

	iTime = pcCircuitSimulation->GetTime();
	for (i = 0; i < miBitWidth; i++)
	{
		pcPort = pcBus->GetPort(pcViewPath, pcCircuitSimulation, i);
		asValues[i] = ReadPortValue(pcComponent, pcPort, iTime);
	}
	AppendTraceValues(psz, asValues, miBitWidth, meRadix, miMaxDigits);
}


//////////////////////////////////////////////////////////////////////////
//
// An IC's ports are logic ports.
//
//////////////////////////////////////////////////////////////////////////
STraceValue CValueView::ReadPortValue(CCircuitComponent* pcComponent, CPort* pcPort, int64 iTime)
{
	return ((CLogicPort*)pcPort)->ReadValue(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
// The value is written as it is in the painter's simulation.
//
//////////////////////////////////////////////////////////////////////////
void CValueView::Paint(CCircuitPainter* pcPainter)
{
	CChars	szValue;

	szValue.Init();
	GetValueText(&szValue, pcPainter->GetViewPath(), pcPainter->GetCircuitSimulation());
	mcShapes.SetText(muiValueText, szValue.Text());
	szValue.Kill();

	CIntegratedCircuitView::Paint(pcPainter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CValueView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CIntegratedCircuitView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("bitWidth", miBitWidth);
	pcWriter->Attribute("radix", RadixToString(meRadix));
	SaveXMLBox(pcWriter, msBoxMinimum, msBoxMaximum);
}


//////////////////////////////////////////////////////////////////////////
//
// The box is loaded rather than measured, so the component keeps the size
// it was made with.
//
//////////////////////////////////////////////////////////////////////////
bool CValueView::LoadValueXML(CMarkupTag* pcTag, SValueViewXML* psValue)
{
	char*	szRadix;

	if (!LoadXMLPlacement(pcTag, &psValue->x, &psValue->y, &psValue->eRotation, &psValue->szName))
	{
		return false;
	}
	if (!LoadXMLFamily(pcTag, &psValue->szFamily, &psValue->bExplicitPowerPorts))
	{
		return false;
	}
	if (!pcTag->GetInt32Attribute("bitWidth", &psValue->iBitWidth) || (psValue->iBitWidth < 1) || (psValue->iBitWidth > RADIX_MAX_BIT_WIDTH))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a bitWidth from 1 to 64.", NULL);
	}
	szRadix = pcTag->GetStringAttribute("radix");
	psValue->eRadix = StringToRadix(szRadix);
	if (psValue->eRadix == RADIX_Cannot)
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has no radix or an unknown one [", (szRadix != NULL) ? szRadix : "", "].", NULL);
	}
	return LoadXMLBox(pcTag, &psValue->sBoxMinimum, &psValue->sBoxMaximum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CValueView::GetBitWidth(void) { return miBitWidth; }
ERadix CValueView::GetRadix(void) { return meRadix; }
void CValueView::GetBox(SInt32Vec2* psBoxMinimum, SInt32Vec2* psBoxMaximum) { *psBoxMinimum = msBoxMinimum; *psBoxMaximum = msBoxMaximum; }


//////////////////////////////////////////////////////////////////////////
//
// A new constant, its box measured to fit its widest value.
//
//////////////////////////////////////////////////////////////////////////
void CConstantView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, CCircuitFonts* pcFonts)
{
	SInt32Vec2	sBoxMinimum;
	SInt32Vec2	sBoxMaximum;

	MeasureBox(pcFonts, iBitWidth, eRadix, &sBoxMinimum, &sBoxMaximum);
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iBitWidth, eRadix, sBoxMinimum, sBoxMaximum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstantView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, SInt32Vec2 sBoxMinimum, SInt32Vec2 sBoxMaximum)
{
	CValueView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iBitWidth, eRadix, sBoxMinimum, sBoxMaximum);
	mcShapes.AddRectangle(msBoxMinimum.x, msBoxMinimum.y, msBoxMaximum.x, msBoxMaximum.y, true, true);
	CreateValuePortViews("Output");
	AddValueText();
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CConstantView::GetType(void)
{
	return "Constant";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CConstantView::GetXMLName(void)
{
	return "Constant";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CConstantView::LoadXML(CMarkupTag* pcTag)
{
	SValueViewXML	sValue;

	if (!LoadValueXML(pcTag, &sValue))
	{
		return false;
	}
	Init(sValue.x, sValue.y, sValue.eRotation, sValue.szName, sValue.szFamily, sValue.bExplicitPowerPorts, sValue.iBitWidth, sValue.eRadix, sValue.sBoxMinimum, sValue.sBoxMaximum);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value starts at 0, as Logicim's.
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CConstantView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	CConstantPins*	pcPins;
	CConstant*		pcConstant;

	pcPins = NewMalloc<CConstantPins>();
	pcPins->Init(pcFamily, miBitWidth);
	pcConstant = NewMalloc<CConstant>();
	pcConstant->Init(pcSimulation, mszName.Text(), pcPins, 0);
	return pcConstant;
}


//////////////////////////////////////////////////////////////////////////
//
// A new probe, its box measured to fit its widest value.
//
//////////////////////////////////////////////////////////////////////////
void CProbeView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, CCircuitFonts* pcFonts)
{
	SInt32Vec2	sBoxMinimum;
	SInt32Vec2	sBoxMaximum;

	MeasureBox(pcFonts, iBitWidth, eRadix, &sBoxMinimum, &sBoxMaximum);
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iBitWidth, eRadix, sBoxMinimum, sBoxMaximum);
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's FrameView with arcs of one grid unit is the box with its
// corners rounded by one.
//
//////////////////////////////////////////////////////////////////////////
void CProbeView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, SInt32Vec2 sBoxMinimum, SInt32Vec2 sBoxMaximum)
{
	SFloat64Vec2	sCentre;

	CValueView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iBitWidth, eRadix, sBoxMinimum, sBoxMaximum);
	if (miMaxDigits == 1)
	{
		sCentre = GetBoxCentre();
		mcShapes.AddCircle(sCentre.x, sCentre.y, (msBoxMaximum.y - msBoxMinimum.y) / 2.0, true, true);
	}
	else
	{
		mcShapes.AddRoundedRectangle(msBoxMinimum.x, msBoxMinimum.y, msBoxMaximum.x, msBoxMaximum.y, 1, true, true);
	}
	CreateValuePortViews("Input");
	AddValueText();
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CProbeView::GetType(void)
{
	return "Probe";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CProbeView::GetXMLName(void)
{
	return "Probe";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CProbeView::LoadXML(CMarkupTag* pcTag)
{
	SValueViewXML	sValue;

	if (!LoadValueXML(pcTag, &sValue))
	{
		return false;
	}
	Init(sValue.x, sValue.y, sValue.eRotation, sValue.szName, sValue.szFamily, sValue.bExplicitPowerPorts, sValue.iBitWidth, sValue.eRadix, sValue.sBoxMinimum, sValue.sBoxMaximum);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CProbeView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	CProbePins*		pcPins;
	CProbe*			pcProbe;

	pcPins = NewMalloc<CProbePins>();
	pcPins->Init(pcFamily, miBitWidth);
	pcProbe = NewMalloc<CProbe>();
	pcProbe->Init(pcSimulation, mszName.Text(), pcPins);
	return pcProbe;
}


//////////////////////////////////////////////////////////////////////////
//
// A new pin, its box measured to fit its widest value.  It's shown on the
// left of an instance of its subcircuit until SetSubcircuitAppearance.
//
//////////////////////////////////////////////////////////////////////////
void CPinView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, CCircuitFonts* pcFonts)
{
	SInt32Vec2	sBoxMinimum;
	SInt32Vec2	sBoxMaximum;

	MeasureBox(pcFonts, iBitWidth, eRadix, &sBoxMinimum, &sBoxMaximum);
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iBitWidth, eRadix, sBoxMinimum, sBoxMaximum);
}


//////////////////////////////////////////////////////////////////////////
//
// The name is a quarter of a grid unit past the end of the box, turned
// with the value.
//
//////////////////////////////////////////////////////////////////////////
void CPinView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, SInt32Vec2 sBoxMinimum, SInt32Vec2 sBoxMaximum)
{
	SFloat64Vec2	sCentre;

	CValueView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iBitWidth, eRadix, sBoxMinimum, sBoxMaximum);
	meAlignment = PA_Left;
	meAnchour = PAN_Centre;
	miWeight = 0;
	mbInverting = false;
	mbClockNotch = false;
	mpcFamily = NULL;

	if (miMaxDigits == 1)
	{
		sCentre = GetBoxCentre();
		mcShapes.AddCircle(sCentre.x, sCentre.y, (msBoxMaximum.y - msBoxMinimum.y) / 2.0, true, true);
	}
	else
	{
		mcShapes.AddRoundedRectangle(msBoxMinimum.x, msBoxMinimum.y, msBoxMaximum.x, msBoxMaximum.y, 1, true, true);
	}
	CreateValuePortViews("Port");
	AddValueText();
	mcShapes.AddText(msBoxMinimum.x - 0.25, 0, szName, CF_SansSerifBold, VALUE_FONT_SIZE, TA_Left, 2);
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPinView::SetSubcircuitAppearance(EPinAlignment eAlignment, EPinAnchour eAnchour, int32 iWeight, bool bInverting, bool bClockNotch)
{
	meAlignment = eAlignment;
	meAnchour = eAnchour;
	miWeight = iWeight;
	mbInverting = bInverting;
	mbClockNotch = bClockNotch;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPinView::GetType(void)
{
	return "Pin";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPinView::GetXMLName(void)
{
	return "Pin";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPinView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CValueView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("alignment", PinAlignmentToString(meAlignment));
	pcWriter->Attribute("anchour", PinAnchourToString(meAnchour));
	pcWriter->Attribute("weight", miWeight);
	pcWriter->Attribute("inverting", mbInverting);
	pcWriter->Attribute("clockNotch", mbClockNotch);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPinView::LoadXML(CMarkupTag* pcTag)
{
	SValueViewXML	sValue;
	EPinAlignment	eAlignment;
	EPinAnchour		eAnchour;
	int32			iWeight;
	bool			bInverting;
	bool			bClockNotch;

	if (!LoadValueXML(pcTag, &sValue))
	{
		return false;
	}
	eAlignment = StringToPinAlignment(pcTag->GetStringAttribute("alignment"));
	eAnchour = StringToPinAnchour(pcTag->GetStringAttribute("anchour"));
	if ((eAlignment == PA_Cannot) || (eAnchour == PAN_Cannot))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has no alignment or anchour or an unknown one.", NULL);
	}
	if (!pcTag->GetInt32Attribute("weight", &iWeight) || !pcTag->GetBoolAttribute("inverting", &bInverting) || !pcTag->GetBoolAttribute("clockNotch", &bClockNotch))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a whole number weight and inverting and clockNotch true or false.", NULL);
	}
	Init(sValue.x, sValue.y, sValue.eRotation, sValue.szName, sValue.szFamily, sValue.bExplicitPowerPorts, sValue.iBitWidth, sValue.eRadix, sValue.sBoxMinimum, sValue.sBoxMaximum);
	SetSubcircuitAppearance(eAlignment, eAnchour, iWeight, bInverting, bClockNotch);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A pin is a passive; unless its power ports are explicit it's powered at
// its family's default voltage, as an IC is.
//
//////////////////////////////////////////////////////////////////////////
CCircuitComponent* CPinView::CreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CFamilyVoltageConfigurationStore* pcFamilies)
{
	CPin*					pcPin;
	CSubcircuitSimulation*	pcSimulation;

	pcSimulation = CanCreateComponent(pcViewPath, pcCircuitSimulation);
	if (pcSimulation == NULL)
	{
		return NULL;
	}

	mpcFamily = pcFamilies->Get(mszFamily.Text());
	if (mpcFamily == NULL)
	{
		gcLogger.Error2(__METHOD__, " Cannot find family [", mszFamily.Text(), "].", NULL);
		return NULL;
	}

	pcPin = NewMalloc<CPin>();
	pcPin->Init(pcSimulation, mszName.Text(), miBitWidth);
	if (!mbExplicitPowerPorts)
	{
		if (!PowerPortsImplicitly(pcSimulation, pcPin->GetVoltageCommon(), pcPin->GetVoltageGround(), mpcFamily))
		{
			return NULL;
		}
	}
	if (!PostCreateComponent(pcViewPath, pcCircuitSimulation, pcPin))
	{
		return NULL;
	}
	return pcPin;
}


//////////////////////////////////////////////////////////////////////////
//
// A pin isn't an IC; see CreateComponent.
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CPinView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentStateXML* CPinView::GetComponentStateXML(void)
{
	return &gcPassiveStateXML;
}


//////////////////////////////////////////////////////////////////////////
//
// The level of the port's trace against the voltage on the pin's VCC, as
// Logicim's PortView.getValue.
//
//////////////////////////////////////////////////////////////////////////
STraceValue CPinView::ReadPortValue(CCircuitComponent* pcComponent, CPort* pcPort, int64 iTime)
{
	STraceValue		sValue;
	CTrace*			pcVCC;
	float32			fVcc;

	sValue.Init(TV_Undriven);
	if ((mpcFamily == NULL) || (pcPort->GetTrace() == NULL))
	{
		return sValue;
	}
	pcVCC = ((CPin*)pcComponent)->GetVoltageCommon()->GetTrace();
	fVcc = (pcVCC != NULL) ? pcVCC->GetVoltage(iTime) : NoVoltage();
	sValue.Init(mpcFamily->GetValue(pcPort->GetTrace()->GetVoltage(iTime), fVcc));
	return sValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPinAlignment CPinView::GetAlignment(void) { return meAlignment; }
EPinAnchour CPinView::GetAnchour(void) { return meAnchour; }
int32 CPinView::GetWeight(void) { return miWeight; }
bool CPinView::IsInverting(void) { return mbInverting; }
bool CPinView::IsClockNotch(void) { return mbClockNotch; }


//////////////////////////////////////////////////////////////////////////
//
// Logicim's names in lower case.
//
//////////////////////////////////////////////////////////////////////////
static const char* gaszPinAlignments[] = { "", "left", "right", "top", "bottom", "not_shown" };
static const char* gaszPinAnchours[] = { "", "negative", "center", "positive" };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* PinAlignmentToString(EPinAlignment eAlignment)
{
	return gaszPinAlignments[eAlignment];
}


//////////////////////////////////////////////////////////////////////////
//
// PA_Cannot if it isn't one PinAlignmentToString gives.
//
//////////////////////////////////////////////////////////////////////////
EPinAlignment StringToPinAlignment(const char* szAlignment)
{
	int		i;

	if (szAlignment != NULL)
	{
		for (i = PA_Left; i <= PA_NotShown; i++)
		{
			if (StringCompare(szAlignment, gaszPinAlignments[i]) == 0)
			{
				return (EPinAlignment)i;
			}
		}
	}
	return PA_Cannot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* PinAnchourToString(EPinAnchour eAnchour)
{
	return gaszPinAnchours[eAnchour];
}


//////////////////////////////////////////////////////////////////////////
//
// PAN_Cannot if it isn't one PinAnchourToString gives.
//
//////////////////////////////////////////////////////////////////////////
EPinAnchour StringToPinAnchour(const char* szAnchour)
{
	int		i;

	if (szAnchour != NULL)
	{
		for (i = PAN_Negative; i <= PAN_Positive; i++)
		{
			if (StringCompare(szAnchour, gaszPinAnchours[i]) == 0)
			{
				return (EPinAnchour)i;
			}
		}
	}
	return PAN_Cannot;
}

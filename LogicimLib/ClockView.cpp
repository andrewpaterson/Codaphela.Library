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
#include "MicroprocessorLib/ClockOscillator.h"
#include "ClockView.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, float64 fFrequency, bool bInverseOut)
{
	SFloat64Vec2	asWave[CLOCK_WAVEFORM_LINES + 1];
	float64			fCentre;
	size			i;

	CIntegratedCircuitView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);
	mfFrequency = fFrequency;
	mbInverseOut = bInverseOut;

	if (!mbInverseOut)
	{
		mcShapes.AddRectangle(-1, -1, 1, 1, true, true);
		fCentre = 0;
	}
	else
	{
		mcShapes.AddRectangle(-0.5, -1, 2.5, 1, true, true);
		fCentre = 1;
	}

	//Low, up, along high, down, along low and up, running up the box.
	asWave[0].Init(fCentre, -0.5);
	asWave[1].Init(fCentre - 0.33, -0.5);
	asWave[2].Init(fCentre - 0.33, 0);
	asWave[3].Init(fCentre + 0.33, 0);
	asWave[4].Init(fCentre + 0.33, 0.5);
	asWave[5].Init(fCentre, 0.5);
	muiFirstWaveformLine = mcShapes.NumShapes();
	for (i = 0; i < CLOCK_WAVEFORM_LINES; i++)
	{
		mcShapes.AddLine(asWave[i].x, asWave[i].y, asWave[i + 1].x, asWave[i + 1].y);
	}

	CreatePowerPortViews();
	AddPortView("Output", 0, -1);
	if (mbInverseOut)
	{
		AddPortView("Output2", 2, -1);
	}
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// The wave is the colour of the clock's voltage, or of a disconnected
// trace without a clock.
//
//////////////////////////////////////////////////////////////////////////
void CClockView::Paint(CCircuitPainter* pcPainter)
{
	ARGB32				sColour;
	size				i;
	CCircuitComponent*	pcComponent;

	pcComponent = GetComponent(pcPainter);
	if ((pcComponent != NULL) && (pcPainter->GetTime() != -1))
	{
		sColour = pcPainter->GetColours()->GetColourForVoltage(((CClockOscillator*)pcComponent)->GetInternalVoltage(pcPainter->GetTime()));
	}
	else
	{
		sColour = pcPainter->GetColours()->msDisconnectedTrace;
	}
	for (i = 0; i < CLOCK_WAVEFORM_LINES; i++)
	{
		mcShapes.SetBorder(muiFirstWaveformLine + i, SC_Explicit, sColour);
	}
	CIntegratedCircuitView::Paint(pcPainter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CClockView::GetType(void)
{
	return "Clock Oscillator";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CClockView::GetXMLName(void)
{
	return "Clock";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClockView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CIntegratedCircuitView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("frequency", mfFrequency);
	pcWriter->Attribute("inverseOut", mbInverseOut);
}


//////////////////////////////////////////////////////////////////////////
//
// The frequency is in Hertz.
//
//////////////////////////////////////////////////////////////////////////
bool CClockView::LoadXML(CMarkupTag* pcTag)
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;
	char*		szFamily;
	bool		bExplicitPowerPorts;
	float64		fFrequency;
	bool		bInverseOut;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName) || !LoadXMLFamily(pcTag, &szFamily, &bExplicitPowerPorts))
	{
		return false;
	}
	if (!pcTag->GetFloat64Attribute("frequency", &fFrequency) || !(fFrequency > 0))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a frequency above 0.", NULL);
	}
	if (!pcTag->GetBoolAttribute("inverseOut", &bInverseOut))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] inverseOut must be true or false.", NULL);
	}
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, fFrequency, bInverseOut);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CClockView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	CClockOscillatorPins*	pcPins;
	CClockOscillator*		pcClock;

	pcPins = NewMalloc<CClockOscillatorPins>();
	pcPins->Init(pcFamily, mbInverseOut);
	pcClock = NewMalloc<CClockOscillator>();
	pcClock->Init(pcSimulation, mszName.Text(), pcPins, (float32)mfFrequency);
	return pcClock;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float64 CClockView::GetFrequency(void) { return mfFrequency; }
bool CClockView::IsInverseOut(void) { return mbInverseOut; }

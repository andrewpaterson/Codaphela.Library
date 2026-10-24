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
#include "MicroprocessorLib/Oscilloscope.h"
#include "MicroprocessorLib/Pins.h"
#include "OscilloscopeView.h"


#define OSCILLOSCOPE_SAMPLE_WIDTH		1.5
#define OSCILLOSCOPE_REFERENCE_WIDTH	1.0
#define OSCILLOSCOPE_FULL_SCALE_VOLTS	8.0


//////////////////////////////////////////////////////////////////////////
//
// Logicim's power ports are never explicit on an oscilloscope.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, int32 iInputCount, int32 iNumberOfDivsWide, int32 iSamplesPerDiv, int32 iDivHeight, float64 fSampleFrequency)
{
	SComponentShape*	psShape;
	float64				fBottom;
	int32				i;
	CChars				szPortName;

	CIntegratedCircuitView::Init(x, y, eRotation, szName, szFamily, false);
	miInputCount = iInputCount;
	miNumberOfDivsWide = iNumberOfDivsWide;
	miSamplesPerDiv = iSamplesPerDiv;
	miDivHeight = iDivHeight;
	mfSampleFrequency = fSampleFrequency;

	fBottom = (miInputCount - 1) * miDivHeight;
	psShape = mcShapes.AddRoundedRectangle(0, -4, miNumberOfDivsWide + 2, fBottom + 2, 1, true, true);
	psShape->eFill = SC_Explicit;
	psShape->sFill = Set32BitColour((uint8)0, (uint8)208, (uint8)208);
	psShape = mcShapes.AddRectangle(0.8, -3, miNumberOfDivsWide + 1.2, fBottom + 1, true, true);
	psShape->eFill = SC_Explicit;
	psShape->sFill = Set32BitColour((uint8)255, (uint8)255, (uint8)255);

	for (i = 0; i < miInputCount; i++)
	{
		szPortName.Init();
		CPins::CalculatePortName(&szPortName, "Input", i, miInputCount);
		AddPortView(szPortName.Text(), 0, i * miDivHeight);
		szPortName.Kill();
	}
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// The samples go over the screen and under the ports.
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeView::Paint(CCircuitPainter* pcPainter)
{
	pcPainter->SetTransform(msPosition, meRotation);
	PaintShapes(pcPainter);
	PaintSamples(pcPainter);
	PaintPorts(pcPainter);
	pcPainter->ClearTransform();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeView::PaintSamples(CCircuitPainter* pcPainter)
{
	COscilloscope*		pcOscilloscope;
	ARGB32				sReference;
	ARGB32				sColour;
	float64				fBottom;
	float64				fTop;
	float64				fTick;
	float64				fX;
	float64				fY;
	size				uiInput;
	size				uiSample;
	size				uiSamples;
	size				uiTick;
	float64				fBehind;
	float64				fColour;
	float32				fMin;
	float32				fMax;

	if (mpcComponent == NULL)
	{
		return;
	}
	pcOscilloscope = (COscilloscope*)mpcComponent;
	uiSamples = pcOscilloscope->GetSampleCount();
	if (uiSamples == 0)
	{
		return;
	}

	sReference = Set32BitColour((uint8)232, (uint8)232, (uint8)232);
	uiTick = pcOscilloscope->GetTickPosition();
	fBottom = (miInputCount - 1) * miDivHeight + 1 - 0.25;
	fTop = -3 + 0.25;
	fTick = 1 + (float64)uiTick / miSamplesPerDiv;
	pcPainter->Line(fTick, fTop, fTick, fBottom, OSCILLOSCOPE_REFERENCE_WIDTH, sReference);

	for (uiInput = 0; uiInput < pcOscilloscope->GetInputCount(); uiInput++)
	{
		fY = (float64)uiInput * miDivHeight;
		pcPainter->Line(1, fY, miNumberOfDivsWide, fY, OSCILLOSCOPE_REFERENCE_WIDTH, sReference);
		for (uiSample = 0; uiSample < uiSamples; uiSample++)
		{
			fMin = pcOscilloscope->GetMinVoltage(uiInput, uiSample);
			fMax = pcOscilloscope->GetMaxVoltage(uiInput, uiSample);
			if (isnan(fMin) || isnan(fMax))
			{
				continue;
			}

			//How far the sample is from being overwritten; the nearest quarter fades to white.
			fBehind = (uiTick >= uiSample) ? (float64)(uiTick - uiSample) : (float64)(uiTick + uiSamples - uiSample);
			fBehind = uiSamples - fBehind;
			sColour = pcPainter->GetColours()->GetColourForVoltage((fMin + fMax) / 2);
			if (fBehind < uiSamples / 4.0)
			{
				fColour = fBehind / (uiSamples / 4.0);
				sColour = Set32BitColour((uint8)(Get8BitRedColour(sColour) * fColour + 255 * (1 - fColour)),
										 (uint8)(Get8BitGreenColour(sColour) * fColour + 255 * (1 - fColour)),
										 (uint8)(Get8BitBlueColour(sColour) * fColour + 255 * (1 - fColour)));
			}

			fX = 1 + (float64)uiSample / miSamplesPerDiv;
			pcPainter->Line(fX, fY - (fMin / OSCILLOSCOPE_FULL_SCALE_VOLTS) * miDivHeight, fX, fY - (fMax / OSCILLOSCOPE_FULL_SCALE_VOLTS) * miDivHeight, OSCILLOSCOPE_SAMPLE_WIDTH, sColour);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* COscilloscopeView::GetType(void)
{
	return "Oscilloscope";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* COscilloscopeView::GetXMLName(void)
{
	return "Oscilloscope";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COscilloscopeView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CComponentView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("family", mszFamily.Text());
	pcWriter->Attribute("inputCount", miInputCount);
	pcWriter->Attribute("numberOfDivsWide", miNumberOfDivsWide);
	pcWriter->Attribute("samplesPerDiv", miSamplesPerDiv);
	pcWriter->Attribute("divHeight", miDivHeight);
	pcWriter->Attribute("sampleFrequency", mfSampleFrequency);
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's limits: 1 to 24 inputs, 4 to 128 divs wide, 4 to 64 samples
// a div and divs 2 to 6 high.
//
//////////////////////////////////////////////////////////////////////////
bool COscilloscopeView::LoadXML(CMarkupTag* pcTag)
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;
	char*		szFamily;
	bool		bExplicitPowerPorts;
	int32		iInputCount;
	int32		iNumberOfDivsWide;
	int32		iSamplesPerDiv;
	int32		iDivHeight;
	float64		fSampleFrequency;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName) || !LoadXMLFamily(pcTag, &szFamily, &bExplicitPowerPorts))
	{
		return false;
	}
	if (!pcTag->GetInt32Attribute("inputCount", &iInputCount) || !pcTag->GetInt32Attribute("numberOfDivsWide", &iNumberOfDivsWide) ||
		!pcTag->GetInt32Attribute("samplesPerDiv", &iSamplesPerDiv) || !pcTag->GetInt32Attribute("divHeight", &iDivHeight) ||
		!pcTag->GetFloat64Attribute("sampleFrequency", &fSampleFrequency))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs whole number inputCount, numberOfDivsWide, samplesPerDiv and divHeight and a sampleFrequency.", NULL);
	}
	if ((iInputCount < 1) || (iInputCount > 24) || (iNumberOfDivsWide < 4) || (iNumberOfDivsWide > 128) ||
		(iSamplesPerDiv < 4) || (iSamplesPerDiv > 64) || (iDivHeight < 2) || (iDivHeight > 6) || !(fSampleFrequency > 0))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has a property out of range.", NULL);
	}
	Init(x, y, eRotation, szName, szFamily, iInputCount, iNumberOfDivsWide, iSamplesPerDiv, iDivHeight, fSampleFrequency);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The oscilloscope's inputs read voltages, not levels; it has no family.
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* COscilloscopeView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	COscilloscopePins*	pcPins;
	COscilloscope*		pcOscilloscope;

	pcPins = NewMalloc<COscilloscopePins>();
	pcPins->Init(miInputCount);
	pcOscilloscope = NewMalloc<COscilloscope>();
	pcOscilloscope->Init(pcSimulation, mszName.Text(), pcPins, (float32)mfSampleFrequency, miNumberOfDivsWide, miSamplesPerDiv);
	return pcOscilloscope;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 COscilloscopeView::GetInputCount(void) { return miInputCount; }
int32 COscilloscopeView::GetNumberOfDivsWide(void) { return miNumberOfDivsWide; }
int32 COscilloscopeView::GetSamplesPerDiv(void) { return miSamplesPerDiv; }
int32 COscilloscopeView::GetDivHeight(void) { return miDivHeight; }
float64 COscilloscopeView::GetSampleFrequency(void) { return mfSampleFrequency; }

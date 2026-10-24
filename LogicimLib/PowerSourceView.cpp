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
#include "MicroprocessorLib/PowerSource.h"
#include "MicroprocessorLib/PowerPinNames.h"
#include "MicroprocessorLib/Voltage.h"
#include "ComponentStateXML.h"
#include "PowerSourceView.h"


//////////////////////////////////////////////////////////////////////////
//
// Subclasses add their shapes, the port view and then CalculateBounds.
//
//////////////////////////////////////////////////////////////////////////
void CPowerSourceView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, float32 fVoltage)
{
	CComponentView::Init(x, y, eRotation, szName);
	mfVoltage = fVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
// A supply has no family.
//
//////////////////////////////////////////////////////////////////////////
CCircuitComponent* CPowerSourceView::CreateComponent(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies)
{
	CPowerSource*	pcPowerSource;

	if (mpcComponent != NULL)
	{
		gcLogger.Error2(__METHOD__, " [", GetType(), "] [", mszName.Text(), "] already has a component.", NULL);
		return NULL;
	}

	pcPowerSource = NewMalloc<CPowerSource>();
	pcPowerSource->Init(pcSimulation, mszName.Text(), mfVoltage);
	if (!ConnectPortViews(pcPowerSource))
	{
		return NULL;
	}
	return pcPowerSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentStateXML* CPowerSourceView::GetComponentStateXML(void)
{
	return &gcPassiveStateXML;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CPowerSourceView::GetVoltage(void)
{
	return mfVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
// Three shrinking bars under a stem down from the port.
//
//////////////////////////////////////////////////////////////////////////
void CGroundView::Init(int32 x, int32 y, ERotation eRotation, const char* szName)
{
	float64		fOffset;

	CPowerSourceView::Init(x, y, eRotation, szName, 0.0f);

	fOffset = 0.5;
	mcShapes.AddLine(-0.9, fOffset, 0.9, fOffset);
	mcShapes.AddLine(-0.6, fOffset + 0.333, 0.6, fOffset + 0.333);
	mcShapes.AddLine(-0.3, fOffset + 0.666, 0.3, fOffset + 0.666);
	mcShapes.AddLine(0, 0, 0, fOffset);
	AddPortView(POWER_PIN_POWER, 0, 0);
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CGroundView::GetType(void)
{
	return "Ground";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CGroundView::GetXMLName(void)
{
	return "Ground";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGroundView::LoadXML(CMarkupTag* pcTag)
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName))
	{
		return false;
	}
	Init(x, y, eRotation, szName);
	return true;
}



//////////////////////////////////////////////////////////////////////////
//
// The box runs from 0.4 below the position to 1 above it, with its point
// at the port.
//
//////////////////////////////////////////////////////////////////////////
void CPositivePowerView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, float32 fVoltage)
{
	SFloat64Vec2		asPoints[5];
	SComponentShape*	psShape;
	CChars				szVoltage;

	CPowerSourceView::Init(x, y, eRotation, szName, fVoltage);

	asPoints[0].Init(0, 1);
	asPoints[1].Init(0.9, 0.4);
	asPoints[2].Init(0.9, -1);
	asPoints[3].Init(-0.9, -1);
	asPoints[4].Init(-0.9, 0.4);
	psShape = mcShapes.AddPolygon(asPoints, 5, true, true);
	if (fVoltage > 4.0f)
	{
		psShape->eFill = SC_DarkShape;
	}

	szVoltage.Init();
	AppendVoltage(&szVoltage, fVoltage, false);
	muiVoltageText = mcShapes.NumShapes();
	mcShapes.AddCentredText(0, -0.1, szVoltage.Text(), CF_SansSerifBold, 11);
	szVoltage.Kill();

	AddPortView(POWER_PIN_POWER, 0, 1);
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// The voltage is written in its trace colour.
//
//////////////////////////////////////////////////////////////////////////
void CPositivePowerView::Paint(CCircuitPainter* pcPainter)
{
	mcShapes.SetBorder(muiVoltageText, SC_Explicit, pcPainter->GetColours()->GetTraceVoltage(mfVoltage));
	CPowerSourceView::Paint(pcPainter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPositivePowerView::GetType(void)
{
	return "Positive Power";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPositivePowerView::GetXMLName(void)
{
	return "PositivePower";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPositivePowerView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CPowerSourceView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("voltage", mfVoltage);
}


//////////////////////////////////////////////////////////////////////////
//
// The voltage is in Volts.
//
//////////////////////////////////////////////////////////////////////////
bool CPositivePowerView::LoadXML(CMarkupTag* pcTag)
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;
	float32		fVoltage;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName))
	{
		return false;
	}
	if (!pcTag->GetFloat32Attribute("voltage", &fVoltage) || !(fVoltage > 0))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a voltage above 0.", NULL);
	}
	Init(x, y, eRotation, szName, fVoltage);
	return true;
}

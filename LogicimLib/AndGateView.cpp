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
#include "MicroprocessorLib/AndGate.h"
#include "AndGateView.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAndGateView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth)
{
	InitAnd(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iInputCount, iInputWidth, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAndGateView::InitAnd(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth, bool bNegated)
{
	CLogicGateView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iInputCount, iInputWidth, bNegated);
	CreateShapes();
	CreateGatePortViews(0);
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// The back is filled first, then the front, then the sides and back are
// outlined so the front's fill doesn't cover them.
//
//////////////////////////////////////////////////////////////////////////
void CAndGateView::CreateShapes(void)
{
	float64		fHeight;
	float64		fBack;

	fHeight = CalculateWidth(miInputCount);
	fBack = 0.5 - 1.4;

	mcShapes.AddRectangle(fBack, -fHeight, 0.5, fHeight, true, false);
	mcShapes.AddArc(0.4, 0, 1.5, fHeight, -90, 180, true, true);
	mcShapes.AddLine(0.5, -fHeight, fBack, -fHeight);
	mcShapes.AddLine(fBack, -fHeight, fBack, fHeight);
	mcShapes.AddLine(fBack, fHeight, 0.5, fHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CAndGateView::GetType(void)
{
	return "AND Gate";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CAndGateView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	if (mbNegated)
	{
		return &OMalloc<CNandGate>(pcSimulation, mszName.Text(), OMalloc<CAndGatePins>(miInputCount * miInputWidth, pcFamily));
	}
	return &OMalloc<CAndGate>(pcSimulation, mszName.Text(), OMalloc<CAndGatePins>(miInputCount * miInputWidth, pcFamily));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNandGateView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth)
{
	InitAnd(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iInputCount, iInputWidth, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CNandGateView::GetType(void)
{
	return "NAND Gate";
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CAndGateView::GetXMLName(void)
{
	return "AndGate";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAndGateView::LoadXML(CMarkupTag* pcTag)
{
	SLogicGateXML	sGate;

	if (!LoadLogicGateXML(pcTag, &sGate))
	{
		return false;
	}
	Init(sGate.x, sGate.y, sGate.eRotation, sGate.szName, sGate.szFamily, sGate.bExplicitPowerPorts, sGate.iInputCount, sGate.iInputWidth);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CNandGateView::GetXMLName(void)
{
	return "NandGate";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNandGateView::LoadXML(CMarkupTag* pcTag)
{
	SLogicGateXML	sGate;

	if (!LoadLogicGateXML(pcTag, &sGate))
	{
		return false;
	}
	Init(sGate.x, sGate.y, sGate.eRotation, sGate.szName, sGate.szFamily, sGate.bExplicitPowerPorts, sGate.iInputCount, sGate.iInputWidth);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAndGateView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	Init(x, y, eRotation, pcProperties->GetText("name", ""), pcProperties->GetText("family", ""), pcProperties->GetBoolean("explicitPowerPorts", mbExplicitPowerPorts), (int)pcProperties->GetInteger("inputCount", miInputCount), (int)pcProperties->GetInteger("inputWidth", miInputWidth));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNandGateView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	Init(x, y, eRotation, pcProperties->GetText("name", ""), pcProperties->GetText("family", ""), pcProperties->GetBoolean("explicitPowerPorts", mbExplicitPowerPorts), (int)pcProperties->GetInteger("inputCount", miInputCount), (int)pcProperties->GetInteger("inputWidth", miInputWidth));
	return true;
}

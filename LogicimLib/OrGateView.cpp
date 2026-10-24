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
#include "MicroprocessorLib/OrGate.h"
#include "MicroprocessorLib/XorGate.h"
#include "OrGateView.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrGateView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth)
{
	InitOr(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iInputCount, iInputWidth, false, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrGateView::InitOr(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth, bool bNegated, bool bExclusive)
{
	CLogicGateView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iInputCount, iInputWidth, bNegated);
	mbExclusive = bExclusive;
	CreateShapes();
	CreateGatePortViews(mbExclusive ? 1 : 0);
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// The body is filled by two pie slices from the side arcs' centres and two
// triangles at the back corners, then the three curves are outlined.
//
//////////////////////////////////////////////////////////////////////////
void COrGateView::CreateShapes(void)
{
	float64			fWidth;
	float64			fRadius;
	float64			fTop;
	float64			fBottom;
	float64			fMiddle;
	float64			fZero;

	fWidth = CalculateWidth(miInputCount);
	fRadius = fWidth * 2;
	fTop = 0.6;
	fBottom = 3.5;
	fMiddle = 0.9;
	fZero = 0.0;

	mcShapes.AddArc(fWidth, fTop, fRadius, 3, OR_ARC_LEFT_START, OR_ARC_SIDE_LENGTH - OR_ARC_FILL_OFFSET, true, false);
	mcShapes.AddArc(-fWidth, fTop, fRadius, 3, OR_ARC_RIGHT_START + OR_ARC_FILL_OFFSET, OR_ARC_SIDE_LENGTH - OR_ARC_FILL_OFFSET, true, false);
	mcShapes.AddTriangle(fWidth, fMiddle, fWidth - OR_FILL_OFFSET, fZero, 0, fZero, true, false);
	mcShapes.AddTriangle(-fWidth, fMiddle, -fWidth + OR_FILL_OFFSET, fZero, 0, fZero, true, false);
	mcShapes.AddArc(fWidth, fTop, fRadius, 3, OR_ARC_LEFT_START, OR_ARC_SIDE_LENGTH, false, true);
	mcShapes.AddArc(-fWidth, fTop, fRadius, 3, OR_ARC_RIGHT_START, OR_ARC_SIDE_LENGTH, false, true);
	mcShapes.AddArc(0, fBottom, fRadius, 3, OR_ARC_BOTTOM_START, OR_ARC_BOTTOM_LENGTH, false, true);
	if (mbExclusive)
	{
		mcShapes.AddArc(0, fBottom + 1, fRadius, 3, OR_ARC_BOTTOM_START, OR_ARC_BOTTOM_LENGTH, false, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* COrGateView::GetType(void)
{
	return "OR Gate";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* COrGateView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	if (mbExclusive)
	{
		CXorGatePins*	pcPins;
		CXorGate*		pcGate;

		pcPins = NewMalloc<CXorGatePins>();
		pcPins->Init(miInputCount * miInputWidth, pcFamily);
		pcGate = mbNegated ? NewMalloc<CXnorGate>() : NewMalloc<CXorGate>();
		pcGate->Init(pcSimulation, mszName.Text(), pcPins);
		return pcGate;
	}
	else
	{
		COrGatePins*	pcPins;
		COrGate*		pcGate;

		pcPins = NewMalloc<COrGatePins>();
		pcPins->Init(miInputCount * miInputWidth, pcFamily);
		pcGate = mbNegated ? NewMalloc<CNorGate>() : NewMalloc<COrGate>();
		pcGate->Init(pcSimulation, mszName.Text(), pcPins);
		return pcGate;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COrGateView::IsExclusive(void)
{
	return mbExclusive;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNorGateView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth)
{
	InitOr(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iInputCount, iInputWidth, true, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CNorGateView::GetType(void)
{
	return "NOR Gate";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXorGateView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth)
{
	InitOr(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iInputCount, iInputWidth, false, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CXorGateView::GetType(void)
{
	return "XOR Gate";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXnorGateView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth)
{
	InitOr(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iInputCount, iInputWidth, true, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CXnorGateView::GetType(void)
{
	return "XNOR Gate";
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* COrGateView::GetXMLName(void)
{
	return "OrGate";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COrGateView::LoadXML(CMarkupTag* pcTag)
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
const char* CNorGateView::GetXMLName(void)
{
	return "NorGate";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNorGateView::LoadXML(CMarkupTag* pcTag)
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
const char* CXorGateView::GetXMLName(void)
{
	return "XorGate";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CXorGateView::LoadXML(CMarkupTag* pcTag)
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
const char* CXnorGateView::GetXMLName(void)
{
	return "XnorGate";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CXnorGateView::LoadXML(CMarkupTag* pcTag)
{
	SLogicGateXML	sGate;

	if (!LoadLogicGateXML(pcTag, &sGate))
	{
		return false;
	}
	Init(sGate.x, sGate.y, sGate.eRotation, sGate.szName, sGate.szFamily, sGate.bExplicitPowerPorts, sGate.iInputCount, sGate.iInputWidth);
	return true;
}


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
	float64		fWidth;
	float64		fBottom;

	fWidth = CalculateWidth(miInputCount);
	fBottom = -0.5 + 1.4;

	mcShapes.AddRectangle(-fWidth, -0.5, fWidth, fBottom, true, false);
	mcShapes.AddArc(0, -0.4, fWidth, 1.5, 0, 180, true, true);
	mcShapes.AddLine(-fWidth, -0.5, -fWidth, fBottom);
	mcShapes.AddLine(-fWidth, fBottom, fWidth, fBottom);
	mcShapes.AddLine(fWidth, fBottom, fWidth, -0.5);
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
	CAndGatePins*	pcPins;
	CAndGate*		pcGate;

	pcPins = NewMalloc<CAndGatePins>();
	pcPins->Init(miInputCount * miInputWidth, pcFamily);
	if (mbNegated)
	{
		pcGate = NewMalloc<CNandGate>();
	}
	else
	{
		pcGate = NewMalloc<CAndGate>();
	}
	pcGate->Init(pcSimulation, mszName.Text(), pcPins);
	return pcGate;
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


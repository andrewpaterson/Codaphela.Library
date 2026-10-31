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
#include "MicroprocessorLib/Buffer.h"
#include "BufferView.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBufferView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iBufferCount, int iBufferWidth)
{
	InitBuffer(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iBufferCount, iBufferWidth, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBufferView::InitBuffer(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iBufferCount, int iBufferWidth, bool bNegated)
{
	CLogicGateView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iBufferCount, iBufferWidth, bNegated);
	CreateShapes();
	CreateBufferPortViews();
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// Narrower triangles when there are more than two.
//
//////////////////////////////////////////////////////////////////////////
void CBufferView::CreateShapes(void)
{
	CArrayInt	aiOffsets;
	size		i;
	float64		fWidth;
	float64		fOffset;

	aiOffsets.Init();
	CalculatePortOffsets(&aiOffsets, miInputCount);
	fWidth = (aiOffsets.NumElements() <= 2) ? 0.75 : 0.5;
	for (i = 0; i < aiOffsets.NumElements(); i++)
	{
		fOffset = aiOffsets.GetValue(i);
		AddBufferShape(&mcShapes, 0, fOffset, fWidth);
	}
	aiOffsets.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A buffer's triangle, fHalfWidth either side of y, from its input at
// x - 1 to its tip at x + 0.9.
//
//////////////////////////////////////////////////////////////////////////
void CBufferView::AddBufferShape(CComponentShapes* pcShapes, float64 x, float64 y, float64 fHalfWidth)
{
	pcShapes->AddTriangle(x + 0.9, y, x - 1, y + fHalfWidth, x - 1, y - fHalfWidth, true, true);
}


//////////////////////////////////////////////////////////////////////////
//
// A buffer is only as wide as its triangles.
//
//////////////////////////////////////////////////////////////////////////
bool CBufferView::IncludeExtraBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds)
{
	return bHasBounds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CBufferView::GetType(void)
{
	return "Buffer";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CBufferView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	if (mbNegated)
	{
		return &OMalloc<CInverter>(pcSimulation, mszName.Text(), OMalloc<CBufferPins>(miInputCount * miInputWidth, pcFamily));
	}
	return &OMalloc<CBuffer>(pcSimulation, mszName.Text(), OMalloc<CBufferPins>(miInputCount * miInputWidth, pcFamily));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInverterView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iBufferCount, int iBufferWidth)
{
	InitBuffer(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iBufferCount, iBufferWidth, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CInverterView::GetType(void)
{
	return "Inverter";
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CBufferView::GetXMLName(void)
{
	return "Buffer";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBufferView::LoadXML(CMarkupTag* pcTag)
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
const char* CInverterView::GetXMLName(void)
{
	return "Inverter";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInverterView::LoadXML(CMarkupTag* pcTag)
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
bool CBufferView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	Init(x, y, eRotation, pcProperties->GetText("name", ""), pcProperties->GetText("family", ""), pcProperties->GetBoolean("explicitPowerPorts", mbExplicitPowerPorts), (int)pcProperties->GetInteger("inputCount", miInputCount), (int)pcProperties->GetInteger("inputWidth", miInputWidth));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInverterView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	Init(x, y, eRotation, pcProperties->GetText("name", ""), pcProperties->GetText("family", ""), pcProperties->GetBoolean("explicitPowerPorts", mbExplicitPowerPorts), (int)pcProperties->GetInteger("inputCount", miInputCount), (int)pcProperties->GetInteger("inputWidth", miInputWidth));
	return true;
}

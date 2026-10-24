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
#include "MicroprocessorLib/Counter.h"
#include "CounterView.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounterView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, uint32 uiTerminalValue)
{
	SPortViewCreator	asBottom[] = {	{ "Clock", "Clock", 1, true, false },
										{ "Reset", "Reset", 1, false, true },
										{ "Load", "Load", 1, false, true },
										{ "Enable", "Enable", 1, false, false },
										{ "Enable Terminal", "Enable Terminal", 1, false, false },
										{ "Data", "Data", (int)iBitWidth, false, false } };
	SPortViewCreator	asTop[] = {		{ "Count", "Count", (int)iBitWidth, false, false },
										{ "Terminal Count", "Terminal Count", 1, false, false } };

	CIntegratedCircuitView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);
	miBitWidth = iBitWidth;
	muiTerminalValue = uiTerminalValue;

	mcShapes.AddRectangle(-9, -6, 9, 6, true, true);
	CreatePowerPortViews();
	CreateLabelledPortViews(asBottom, 6, asTop, 2, 6, -6, -6, 4, 2);
	mcShapes.AddText(-8, 0, "Up Counter", CF_SansSerifBold, PORT_LABEL_FONT_SIZE, TA_Centre);
	muiValueText = AddDetail(8, 4, "Value", "0");
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// The count is shown once there's a counter, and kept after.
//
//////////////////////////////////////////////////////////////////////////
void CCounterView::Paint(CCircuitPainter* pcPainter)
{
	CChars				szValue;
	CCircuitComponent*	pcComponent;

	pcComponent = GetComponent(pcPainter);
	if (pcComponent != NULL)
	{
		szValue.Init();
		((CCounter*)pcComponent)->AppendCounterValueHex(&szValue);
		mcShapes.SetText(muiValueText, szValue.Text());
		szValue.Kill();
	}
	CIntegratedCircuitView::Paint(pcPainter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCounterView::GetType(void)
{
	return "Counter";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCounterView::GetXMLName(void)
{
	return "Counter";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounterView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CIntegratedCircuitView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("bitWidth", miBitWidth);
	pcWriter->Attribute("terminalValue", (int64)muiTerminalValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCounterView::LoadXML(CMarkupTag* pcTag)
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;
	char*		szFamily;
	bool		bExplicitPowerPorts;
	int32		iBitWidth;
	int64		iTerminalValue;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName) || !LoadXMLFamily(pcTag, &szFamily, &bExplicitPowerPorts))
	{
		return false;
	}
	if (!pcTag->GetInt32Attribute("bitWidth", &iBitWidth) || (iBitWidth < 1) || (iBitWidth > 32))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a bitWidth from 1 to 32.", NULL);
	}
	if (!pcTag->GetInt64Attribute("terminalValue", &iTerminalValue) || (iTerminalValue < 0) || (iTerminalValue > 0xFFFFFFFFLL))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a whole number terminalValue.", NULL);
	}
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iBitWidth, (uint32)iTerminalValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CCounterView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	CCounterPins*	pcPins;
	CCounter*		pcCounter;

	pcPins = NewMalloc<CCounterPins>();
	pcPins->Init(pcFamily, miBitWidth);
	pcCounter = NewMalloc<CCounter>();
	pcCounter->Init(pcSimulation, mszName.Text(), pcPins, miBitWidth, muiTerminalValue);
	return pcCounter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CCounterView::GetBitWidth(void) { return miBitWidth; }
uint32 CCounterView::GetTerminalValue(void) { return muiTerminalValue; }

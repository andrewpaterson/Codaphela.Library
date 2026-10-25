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
#include "MicroprocessorLib/DTypeFlipFlop.h"
#include "FlipFlopView.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlopView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts)
{
	SPortViewCreator	asLeft[] = {		{ "CP", "CP", 1, true, false },
										{ "SD", "SD", 1, false, true },
										{ "D", "D", 1, false, false },
										{ "RD", "RD", 1, false, true } };
	SPortViewCreator	asRight[] = {	{ "Q", "Q", 1, false, false },
										{ "Q", "QB", 1, false, true } };

	CIntegratedCircuitView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);
	mcShapes.AddRectangle(-3, -4, 3, 4, true, true);
	CreatePowerPortViews();
	CreateLabelledPortViews(asLeft, 4, asRight, 2, -3, 3, -3, -1, 2);
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CDTypeFlipFlopView::GetType(void)
{
	return "D Flip Flop";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CDTypeFlipFlopView::GetXMLName(void)
{
	return "DTypeFlipFlop";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDTypeFlipFlopView::LoadXML(CMarkupTag* pcTag)
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;
	char*		szFamily;
	bool		bExplicitPowerPorts;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName) || !LoadXMLFamily(pcTag, &szFamily, &bExplicitPowerPorts))
	{
		return false;
	}
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CDTypeFlipFlopView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	CDTypeFlipFlopPins*		pcPins;
	CDTypeFlipFlop*			pcFlipFlop;

	pcPins = NewMalloc<CDTypeFlipFlopPins>();
	pcPins->Init(pcFamily);
	pcFlipFlop = NewMalloc<CDTypeFlipFlop>();
	pcFlipFlop->Init(pcSimulation, mszName.Text(), pcPins);
	return pcFlipFlop;
}

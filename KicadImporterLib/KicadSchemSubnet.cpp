/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "KicadSchemSubnet.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SKicadSchemNetPin::Init(CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin, SKicadSchemPoint* psPosition)
{
	this->pcSymbol = pcSymbol;
	this->pcPin = pcPin;
	sPosition = *psPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SKicadSchemNetSheetPin::Init(CKicadSchemSheet* pcSheet, CKicadSchemSheetPin* pcPin)
{
	this->pcSheet = pcSheet;
	this->pcPin = pcPin;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnet::Init(void)
{
	mapcWires.Init();
	masPins.Init();
	mapcLabels.Init();
	masSheetPins.Init();
	mapcJunctions.Init();
	mbNoConnect = false;
	mpcNet = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnet::Kill(void)
{
	mpcNet = NULL;
	mapcJunctions.Kill();
	masSheetPins.Kill();
	mapcLabels.Kill();
	masPins.Kill();
	mapcWires.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnet::AddWire(CKicadSchemWire* pcWire)
{
	mapcWires.Add(pcWire);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnet::AddPin(CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin, SKicadSchemPoint* psPosition)
{
	SKicadSchemNetPin*	psPin;

	psPin = masPins.Add();
	psPin->Init(pcSymbol, pcPin, psPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnet::AddLabel(CKicadSchemLabel* pcLabel)
{
	mapcLabels.Add(pcLabel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnet::AddSheetPin(CKicadSchemSheet* pcSheet, CKicadSchemSheetPin* pcPin)
{
	SKicadSchemNetSheetPin*	psPin;

	psPin = masSheetPins.Add();
	psPin->Init(pcSheet, pcPin);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnet::AddJunction(CKicadSchemJunction* pcJunction)
{
	mapcJunctions.Add(pcJunction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnet::SetNoConnect(void)
{
	mbNoConnect = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnet::SetNet(CKicadSchemNet* pcNet)
{
	mpcNet = pcNet;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the symbol's pin isn't in the subnet.
//
//////////////////////////////////////////////////////////////////////////
SKicadSchemNetPin* CKicadSchemSubnet::GetPin(CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin)
{
	size				i;
	SKicadSchemNetPin*	psPin;

	for (i = 0; i < masPins.NumElements(); i++)
	{
		psPin = masPins.Get(i);
		if ((psPin->pcSymbol == pcSymbol) && (psPin->pcPin == pcPin))
		{
			return psPin;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemSubnet::NumWires(void) { return mapcWires.NumElements(); }
CKicadSchemWire* CKicadSchemSubnet::GetWire(size uiIndex) { return mapcWires.SafeGetPtr(uiIndex); }
size CKicadSchemSubnet::NumPins(void) { return masPins.NumElements(); }
SKicadSchemNetPin* CKicadSchemSubnet::GetPin(size uiIndex) { return masPins.SafeGet(uiIndex); }
size CKicadSchemSubnet::NumLabels(void) { return mapcLabels.NumElements(); }
CKicadSchemLabel* CKicadSchemSubnet::GetLabel(size uiIndex) { return mapcLabels.SafeGetPtr(uiIndex); }
size CKicadSchemSubnet::NumSheetPins(void) { return masSheetPins.NumElements(); }
SKicadSchemNetSheetPin* CKicadSchemSubnet::GetSheetPin(size uiIndex) { return masSheetPins.SafeGet(uiIndex); }
size CKicadSchemSubnet::NumJunctions(void) { return mapcJunctions.NumElements(); }
CKicadSchemJunction* CKicadSchemSubnet::GetJunction(size uiIndex) { return mapcJunctions.SafeGetPtr(uiIndex); }
bool CKicadSchemSubnet::IsNoConnect(void) { return mbNoConnect; }
CKicadSchemNet* CKicadSchemSubnet::GetNet(void) { return mpcNet; }


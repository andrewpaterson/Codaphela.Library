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
#ifndef __KICAD_SCHEM_SUBNET_H__
#define __KICAD_SCHEM_SUBNET_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "KicadSchemSymbol.h"
#include "KicadSchemLibPin.h"
#include "KicadSchemWire.h"
#include "KicadSchemJunction.h"
#include "KicadSchemLabel.h"
#include "KicadSchemSheet.h"


class CKicadSchemNet;


//A pin of a placed symbol and where it is on the sheet.
struct SKicadSchemNetPin
{
	CKicadSchemSymbol*	pcSymbol;
	CKicadSchemLibPin*	pcPin;
	SKicadSchemPoint	sPosition;

	void	Init(CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin, SKicadSchemPoint* psPosition);
};


//A pin on the edge of a sheet symbol.
struct SKicadSchemNetSheetPin
{
	CKicadSchemSheet*		pcSheet;
	CKicadSchemSheetPin*	pcPin;

	void	Init(CKicadSchemSheet* pcSheet, CKicadSchemSheetPin* pcPin);
};


typedef CArrayTemplate<SKicadSchemNetPin>			CArrayKicadSchemNetPin;
typedef CArrayTemplate<SKicadSchemNetSheetPin>		CArrayKicadSchemNetSheetPin;
typedef CArrayTemplatePtr<CKicadSchemWire>			CArrayKicadSchemWirePtr;
typedef CArrayTemplatePtr<CKicadSchemLabel>			CArrayKicadSchemLabelPtr;
typedef CArrayTemplatePtr<CKicadSchemJunction>		CArrayKicadSchemJunctionPtr;


//The things on a sheet that its wires join together: KiCad's connection subgraph.  Pins, labels and sheet pins join
//the wires they're on (at an end or anywhere along them) and things at the same point join each other.  A pin on
//nothing is a subnet of its own.  Subnets are joined into nets by name (see CKicadSchemNet), so each is the part of
//a net that is drawn as wires.  Buses are not part of any subnet.
class CKicadSchemSubnet
{
protected:
	CArrayKicadSchemWirePtr			mapcWires;
	CArrayKicadSchemNetPin			masPins;
	CArrayKicadSchemLabelPtr		mapcLabels;
	CArrayKicadSchemNetSheetPin		masSheetPins;
	CArrayKicadSchemJunctionPtr		mapcJunctions;
	bool							mbNoConnect;
	CKicadSchemNet*					mpcNet;

public:
	void					Init(void);
	void					Kill(void);

	void					AddWire(CKicadSchemWire* pcWire);
	void					AddPin(CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin, SKicadSchemPoint* psPosition);
	void					AddLabel(CKicadSchemLabel* pcLabel);
	void					AddSheetPin(CKicadSchemSheet* pcSheet, CKicadSchemSheetPin* pcPin);
	void					AddJunction(CKicadSchemJunction* pcJunction);
	void					SetNoConnect(void);
	void					SetNet(CKicadSchemNet* pcNet);

	size					NumWires(void);
	CKicadSchemWire*		GetWire(size uiIndex);
	size					NumPins(void);
	SKicadSchemNetPin*		GetPin(size uiIndex);
	SKicadSchemNetPin*		GetPin(CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin);
	size					NumLabels(void);
	CKicadSchemLabel*		GetLabel(size uiIndex);
	size					NumSheetPins(void);
	SKicadSchemNetSheetPin*	GetSheetPin(size uiIndex);
	size					NumJunctions(void);
	CKicadSchemJunction*	GetJunction(size uiIndex);
	bool					IsNoConnect(void);
	CKicadSchemNet*			GetNet(void);
};


#endif // __KICAD_SCHEM_SUBNET_H__


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
#ifndef __KICAD_SCHEM_LOGICIM_CONVERTER_H__
#define __KICAD_SCHEM_LOGICIM_CONVERTER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayChars.h"
#include "LogicimLib/CircuitViews.h"
#include "LogicimLib/CircuitFonts.h"
#include "KicadSchemNetlist.h"
#include "KicadSchemLogicFunction.h"
#include "KicadSchemSubnetGraph.h"


//KiCad's 2.54mm (100 mil) pin spacing is one Logicim grid unit.
#define KICAD_SCHEM_LOGICIM_GRID			2.54

//The gap kept between components' boxes.
#define KICAD_SCHEM_LOGICIM_SPACING			2

//How far a component is moved to keep it clear of the others before giving up.
#define KICAD_SCHEM_LOGICIM_SEPARATE_RADIUS	400


//A component made in the Logicim circuit and where KiCad has it, in grid units.  The anchor is the point of the view
//that goes there (a power symbol's port, otherwise the view's position), from its position.  A tunnel or power
//supply has one port; if KiCad has it right on a pin it's attached to that pin's port: it goes port to port with it
//(Logicim joins ports in the same place), pointing away, and moves with it.
struct SKicadSchemLogicimItem
{
	CComponentView*		pcView;
	SFloat64Vec2		sKicadPosition;
	SInt32Vec2			sAnchor;
	CPortView*			pcPort;
	CPortView*			pcAttachedTo;
};


//A port in the Logicim circuit standing for a pin, power symbol or label of a subnet.
struct SKicadSchemLogicimTerminal
{
	CKicadSchemSubnet*	pcSubnet;
	SKicadSchemPoint	sKicadPosition;
	CPortView*			pcPortView;
	size				uiItem;
};


//Two terminals of a subnet to be wired, the from terminal to the to terminal (already wired to the rest of the
//subnet), with the bend KiCad's wire between them has.
struct SKicadSchemLogicimConnection
{
	size		uiSubnet;
	size		uiFrom;
	size		uiTo;
	float64		fBend;
	bool		bConnected;
};


typedef CArrayTemplate<SKicadSchemLogicimItem>			CArrayKicadSchemLogicimItem;
typedef CArrayTemplate<SKicadSchemLogicimTerminal>		CArrayKicadSchemLogicimTerminal;
typedef CArrayTemplate<SKicadSchemLogicimConnection>	CArrayKicadSchemLogicimConnection;


//Makes a Logicim circuit from one KiCad sheet.
//
//Each symbol becomes the components Logicim has for it.  A 74 series gate part (see ParseKicadSchemLogicFunction)
//becomes a gate for each of its gates, found by its pin names ("1A", "1B" and "1Y"; "A" and "Y" on a single gate),
//(or, if its pins have no names, by their types) and a 74 flip flop a D flip flop for each of its flip flops ("1CP", "1D", "~{1SD}", "~{1RD}", "1Q" and "~{1Q}").
//Their power pins are left out: the gates have implied power.  A power symbol becomes a ground or a positive supply
//of its voltage ("+3V3" is 3.3V).  Resistors, capacitors, inductors, diodes, LEDs, mounting holes, net ties, test
//points and power flags are left out.  Any other symbol becomes an instance of a placeholder subcircuit made for its
//library symbol and unit: an empty circuit with a pin for each of its shown pins, on the side KiCad draws it, so it
//can be wired and its logic filled in later.  Hierarchical sheets are left out.
//
//A subnet that is part of a bigger net (it's joined to the rest by labels) gets a tunnel named as the net at each of
//its labels, so each subnet is wired on its own, as KiCad drew it.  The terminals of each subnet are wired as a tree
//in the order KiCad's wires join them, each connection hinted to bend as KiCad's wire first bends.  Connections
//between the same two components are routed as a run (see CTraceAutoConnector).
//
//The components are first placed spread out from where KiCad has them (like raisins in rising dough), wired, and
//then moved back towards KiCad's places in steps and rewired at each, while the wiring stays as good.  The circuit
//is left at the smallest spread that wired as many connections as the first.
class CKicadSchemLogicimConverter
{
protected:
	CKicadSchemFile*					mpcFile;
	CKicadSchemNetlist*					mpcNetlist;
	CCircuitViews*						mpcCircuits;
	CCircuitFonts*						mpcFonts;
	CCircuitView*						mpcCircuit;
	CArrayKicadSchemLogicimItem			masItems;
	CArrayKicadSchemLogicimTerminal		masTerminals;
	CArrayKicadSchemLogicimConnection	masConnections;
	float64								mfSpread;
	size								muiUnconnected;
	size								muiGates;
	size								muiPlaceholders;
	size								muiPowers;
	size								muiTunnels;
	size								muiSkipped;
	size								muiOverlaps;

public:
	void							Init(CKicadSchemFile* pcFile, CKicadSchemNetlist* pcNetlist, CCircuitViews* pcCircuits, CCircuitFonts* pcFonts);
	void							Kill(void);

	bool							Convert(const char* szTypeName);

	CCircuitView*					GetCircuit(void);
	float64							GetSpread(void);
	size							NumConnections(void);
	size							NumUnconnected(void);
	SKicadSchemLogicimConnection*	GetConnection(size uiIndex);
	size							NumTerminals(void);
	SKicadSchemLogicimTerminal*		GetTerminal(size uiIndex);
	size							NumItems(void);
	size							NumGates(void);
	size							NumPlaceholders(void);
	size							NumPowers(void);
	size							NumTunnels(void);
	size							NumSkipped(void);
	size							NumOverlaps(void);

	static bool						IsSkippedSymbol(CKicadSchemSymbol* pcSymbol);
	static bool						IsGroundName(const char* szName);
	static float32					GetSupplyVoltage(const char* szName);
	static ERotation				GetKicadRotation(ERotation eUpright, float64 fAngle);

protected:
	bool							AddSymbol(CKicadSchemSymbol* pcSymbol);
	bool							AddPower(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol);
	bool							AddLogicParts(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol, EKicadSchemLogicFunction eFunction, const char* szFamily);
	bool							AddPlaceholder(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol);
	CCircuitView*					GetPlaceholderCircuit(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol);
	void							GetPlaceholderPins(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol, CArrayTemplatePtr<CKicadSchemLibPin>* papcPins, CArrayChars* paszNames);
	void							AddTunnels(void);
	void							AddConnections(void);
	void							AddSubnetConnections(CKicadSchemSubnet* pcSubnet, size uiSubnet);

	size							AddItem(CComponentView* pcView, SKicadSchemPoint* psKicadPosition, CPortView* pcAnchor);
	void							AttachItems(void);
	void							PlaceAttachedItems(void);
	bool							GetItemBox(size uiItem, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum);
	void							AddTerminal(CKicadSchemSubnet* pcSubnet, SKicadSchemPoint* psKicadPosition, CPortView* pcPortView, size uiItem);
	void							AddPinTerminal(CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin, CPortView* pcPortView, size uiItem);

	void							Place(float64 fSpread);
	void							SeparateItems(void);
	size							Route(void);
};


#endif // __KICAD_SCHEM_LOGICIM_CONVERTER_H__


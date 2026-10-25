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

//Knots are wire turns and crossings.  An area of DENSE_WINDOW grid units square with more than DENSE_KNOTS is dense,
//and is exploded: EXPLODE_SPACE grid units are put in across it each way.
#define KICAD_SCHEM_LOGICIM_DENSE_WINDOW	12
#define KICAD_SCHEM_LOGICIM_DENSE_KNOTS		24
#define KICAD_SCHEM_LOGICIM_DENSE_AREAS		3
#define KICAD_SCHEM_LOGICIM_DENSE_SCAN		12
#define KICAD_SCHEM_LOGICIM_EXPLODE_SPACE	4
#define KICAD_SCHEM_LOGICIM_EXPLODE_PASSES	4


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
	SInt32Vec2			sOffset;
	CPortView*			pcPort;
	CPortView*			pcAttachedTo;
};


//A port in the Logicim circuit standing for a pin, power symbol or label of a subnet, or (bBus) a bus's splitter or
//tunnel, which carries all its members.
struct SKicadSchemLogicimTerminal
{
	CKicadSchemSubnet*	pcSubnet;
	SKicadSchemPoint	sKicadPosition;
	CPortView*			pcPortView;
	size				uiItem;
	bool				bBus;
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
//becomes a gate for each of its gates, found by its pin names ("1A", "1B" and "1Y"; "A" and "Y" on a single gate) or,
//if its pins have no names, by their types; and a 74 flip flop becomes a D flip flop for each of its flip flops ("1CP",
//"1D", "~{1SD}", "~{1RD}", "1Q" and "~{1Q}").  Their power pins are left out: the gates have implied power.  An
//inverter part is placed as KiCad has it: each unit of a part drawn as units (a 74LVC04's "_Multi" symbol) is an
//inverter unit, and a part drawn whole is an inverter package of all its inverters; a single inverter is an
//inverter.  A power symbol becomes a ground or a positive supply of its voltage ("+3V3" is 3.3V).  Resistors,
//capacitors, inductors, diodes, LEDs, mounting holes, net ties, test points and power flags are left out.  Any other
//symbol becomes an instance of a placeholder subcircuit made for its library symbol and unit: an empty circuit with a
//pin for each of its shown pins, on the side KiCad draws it, so it can be wired and its logic filled in
//later.  Hierarchical sheets are left out.
//
//A subnet that is part of a bigger net (it's joined to the rest by labels) gets a tunnel named as the net at each of
//its labels, so each subnet is wired on its own, as KiCad drew it.  A tunnel or supply KiCad has right on a pin is
//attached to the pin's port: port to port, pointing away from it, and moving with it.  The terminals of each subnet are
//wired as a tree in the order KiCad's wires join them, each connection hinted to bend as KiCad's wire first
//bends.  Connections between the same two components are routed as a run (see CTraceAutoConnector).
//
//A bus becomes a bus of its members in Logicim.  Each run of its entries on one side of it (a column of entries
//tapping a bus going up and down) becomes a splitter, its ends in the order KiCad's entries are, joined to the
//members' wires.  A splitter has an end for each member it taps and, if the bus has others, an end for the rest.  The
//splitters of a bus are wired to each other along it, and a bus whose name is on other buses gets a tunnel at each of
//its labels.
//
//The components are first placed spread out from where KiCad has them (like raisins in rising dough), wired, and
//then moved back towards KiCad's places in steps and rewired at each, while the wiring stays as good; between the
//last spread that wired as well and the first that didn't, two halfway spreads are tried.  The circuit is left at
//the smallest spread that wired as many connections as the first.
//
//Then dense areas, where the wires knot (turn and cross a lot), are exploded a few times: space is put in across
//each one, up and down and left and right (so what's lined up stays lined up), and it's rewired.  An explosion is
//kept if it wires as well and leaves fewer knots over the limit in the dense areas.
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
	size								muiSplitters;
	size								muiSkipped;
	size								muiOverlaps;
	size								muiExplosions;
	size								muiExcessBefore;
	size								muiExcessAfter;

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
	size							NumSplitters(void);
	size							NumSkipped(void);
	size							NumOverlaps(void);
	size							NumExplosions(void);
	size							GetExcessKnotsBefore(void);
	size							GetExcessKnotsAfter(void);
	size							FindDenseAreas(CArrayTemplate<SFloat64Vec2>* pasCentres);

	static bool						IsSkippedSymbol(CKicadSchemSymbol* pcSymbol);
	static bool						IsGroundName(const char* szName);
	static float32					GetSupplyVoltage(const char* szName);
	static ERotation				GetKicadRotation(ERotation eUpright, float64 fAngle);

protected:
	bool							AddSymbol(CKicadSchemSymbol* pcSymbol);
	bool							AddPower(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol);
	bool							AddLogicParts(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol, EKicadSchemLogicFunction eFunction, const char* szFamily, const char* szPartType);
	void							AddInverterPackage(CKicadSchemSymbol* pcSymbol, CArrayChars* paszIndices, CArrayTemplatePtr<CKicadSchemLibPin>* papcPins, CArrayInt* paiGroups, CArrayChars* paszLetters, CArrayKicadSchemPoint* pasPositions, const char* szFamily, const char* szPartType);
	bool							AddPlaceholder(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol);
	CCircuitView*					GetPlaceholderCircuit(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol);
	void							GetPlaceholderPins(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol, CArrayTemplatePtr<CKicadSchemLibPin>* papcPins, CArrayChars* paszNames);
	void							AddTunnels(void);
	void							AddBuses(void);
	void							AddBus(CKicadSchemBus* pcBus, size uiBus);
	void							AddBusSplitter(CKicadSchemBus* pcBus, CArrayInt* paiTaps, bool bVertical, int iSide);
	void							AddConnections(void);
	void							AddSubnetConnections(CKicadSchemSubnet* pcSubnet, size uiSubnet);

	size							AddItem(CComponentView* pcView, SKicadSchemPoint* psKicadPosition, CPortView* pcAnchor);
	void							AttachItems(void);
	void							PlaceAttachedItems(void);
	bool							GetItemBox(size uiItem, SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum);
	void							AddTerminal(CKicadSchemSubnet* pcSubnet, SKicadSchemPoint* psKicadPosition, CPortView* pcPortView, size uiItem, bool bBus = false);
	void							AddPinTerminal(CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin, CPortView* pcPortView, size uiItem);

	void							Place(float64 fSpread);
	void							Explode(void);
	void							ExplodeAreas(CArrayTemplate<SFloat64Vec2>* pasCentres);
	void							SeparateItems(void);
	size							Route(void);
};


#endif // __KICAD_SCHEM_LOGICIM_CONVERTER_H__


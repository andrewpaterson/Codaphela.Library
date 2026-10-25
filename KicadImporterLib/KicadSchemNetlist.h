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
#ifndef __KICAD_SCHEM_NETLIST_H__
#define __KICAD_SCHEM_NETLIST_H__
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ListVariable.h"
#include "KicadSchemFile.h"
#include "KicadSchemNet.h"
#include "KicadSchemBus.h"


enum EKicadSchemNetNodeType
{
	KSNNT_Wire,
	KSNNT_Pin,
	KSNNT_Label,
	KSNNT_SheetPin,
	KSNNT_Junction,
	KSNNT_NoConnect
};


//One thing on the sheet that can be connected, while the netlist is built.  A pin's owner is its symbol and a sheet
//pin's is its sheet.
struct SKicadSchemNetNode
{
	EKicadSchemNetNodeType	eType;
	void*					pvItem;
	void*					pvOwner;
	SKicadSchemPoint		sPosition;
};


typedef CArrayTemplate<SKicadSchemNetNode>	CArrayKicadSchemNetNode;


//The connectivity of one sheet, as KiCad's connection graph.  Build finds the subnets (what the wires join) and then
//joins them into nets by name.
//
//Local and hierarchical labels name nets on this sheet; global labels, power symbols and hidden power input pins
//name nets on every sheet (KiCad's SCH_PIN::IsGlobalPower: a power input pin that is hidden or on a power symbol).
//A power symbol names its net by its Value and a hidden power input pin by its pin name.  On one sheet every name is
//the same net whatever its scope: KiCad joins a local label to the global label or power symbol with its text.  A net
//with no name gets one from its first pin, as KiCad's "Net-(U1-Pad3)".  A pin common to all the units of an annotated
//symbol is placed with each unit and joins the subnets of its places.
//
//Buses are kept apart: the wires of a bus's members are joined by their labels, as KiCad joins them.  The buses
//themselves (see CKicadSchemBus) are found after the nets, each with the labels on it and the entries tapping it.
//A bus's labels name a subnet of their own too.  Buses with the same name are one bus net (not one of the nets).
//Hierarchical sheets are not followed; a sheet pin is part of the subnet it is wired to.
class CKicadSchemNetlist
{
protected:
	CKicadSchemFile*			mpcFile;
	CListVariable				macSubnets;
	CListVariable				macNets;
	CListVariable				macBuses;
	CListVariable				macBusNets;
	CArrayKicadSchemNetNode		masNodes;
	CArrayInt					maiParents;

public:
	void				Init(CKicadSchemFile* pcFile);
	void				Kill(void);

	bool				Build(void);

	size				NumSubnets(void);
	CKicadSchemSubnet*	GetSubnet(size uiIndex);
	size				NumNets(void);
	CKicadSchemNet*		GetNet(size uiIndex);
	CKicadSchemNet*		GetNet(char* szName);
	CKicadSchemSubnet*	GetPinSubnet(CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin);
	CKicadSchemSubnet*	GetPinSubnet(char* szReference, char* szPinNumber);
	CKicadSchemNet*		GetPinNet(char* szReference, char* szPinNumber);
	size				NumBuses(void);
	CKicadSchemBus*		GetBus(size uiIndex);
	CKicadSchemBus*		GetBus(char* szName);
	size				NumBusNets(void);
	CKicadSchemNet*		GetBusNet(size uiIndex);
	bool				IsBusLabel(CKicadSchemLabel* pcLabel);
	CKicadSchemSubnet*	GetSubnetAt(SKicadSchemPoint* psPoint);

	static bool			IsGlobalPowerPin(CKicadSchemLibSymbol* pcLibSymbol, CKicadSchemLibPin* pcPin);
	static bool			IsSharedPin(CKicadSchemLibSymbol* pcLibSymbol, CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin);
	static char*		GetGlobalPowerName(CKicadSchemSymbol* pcSymbol, CKicadSchemLibSymbol* pcLibSymbol, CKicadSchemLibPin* pcPin);

protected:
	bool				AddNodes(void);
	SKicadSchemNetNode*	AddNode(EKicadSchemNetNodeType eType, void* pvItem, void* pvOwner, SKicadSchemPoint* psPosition);
	void				JoinNodes(void);
	void				CreateSubnets(void);
	void				CreateNets(void);
	void				AddSubnetNames(CKicadSchemSubnet* pcSubnet, size uiSubnet, CArrayChars* paszNames, CArrayInt* paiNameSubnets, CArrayInt* paiSubnetParents);
	void				AddSubnetName(char* szScope, char* szName, size uiSubnet, CArrayChars* paszNames, CArrayInt* paiNameSubnets, CArrayInt* paiSubnetParents);
	void				NameNet(CKicadSchemNet* pcNet);
	void				CreateBuses(void);
	void				AddBusTaps(void);
	void				AddBusMembers(void);
	bool				IsWireNode(SKicadSchemNetNode* psNode);
};


int		FindKicadSchemRoot(CArrayInt* paiParents, int iNode);
void	JoinKicadSchemRoots(CArrayInt* paiParents, int iNode1, int iNode2);


#endif // __KICAD_SCHEM_NETLIST_H__


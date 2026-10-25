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
#ifndef __KICAD_SCHEM_BUS_H__
#define __KICAD_SCHEM_BUS_H__
#include "BaseLib/ArrayChars.h"
#include "KicadSchemBusEntry.h"
#include "KicadSchemNet.h"


//Where a bus entry joins a member's wire to a bus: the entry's end on the bus and its end on the member's wire, and
//the subnet of that wire.  bVertical is whether the bus runs up and down where the entry is on it.
struct SKicadSchemBusTap
{
	CKicadSchemBusEntry*	pcEntry;
	SKicadSchemPoint		sBusPoint;
	SKicadSchemPoint		sWirePoint;
	CKicadSchemSubnet*		pcMember;
	bool					bVertical;
};


typedef CArrayTemplate<SKicadSchemBusTap>	CArrayKicadSchemBusTap;


//The bus wires that touch each other, as a subnet of its own (its wires and the junctions on them), the labels on
//them and the bus entries joining members to it.  A bus is named by its first label, and buses with the same name
//are one bus with the same members, in the same order: the members its labels name ("D[0..7]" is D0 to D7) and then
//the nets of the members tapped by its entries that the labels don't name.  An entry on a member that isn't in a net
//(nothing is wired to it) isn't a tap.
class CKicadSchemBus
{
protected:
	CKicadSchemSubnet			mcSubnet;
	CArrayKicadSchemLabelPtr	mapcLabels;
	CArrayKicadSchemBusTap		masTaps;
	CArrayChars					maszMembers;

public:
	void					Init(void);
	void					Kill(void);

	void					AddWire(CKicadSchemWire* pcWire);
	void					AddJunction(CKicadSchemJunction* pcJunction);
	void					AddLabel(CKicadSchemLabel* pcLabel);
	void					AddTap(CKicadSchemBusEntry* pcEntry, SKicadSchemPoint* psBusPoint, SKicadSchemPoint* psWirePoint, CKicadSchemSubnet* pcMember, bool bVertical);
	void					AddMember(char* szMember);

	CKicadSchemSubnet*		GetSubnet(void);
	char*					GetName(void);
	size					NumLabels(void);
	CKicadSchemLabel*		GetLabel(size uiIndex);
	size					NumTaps(void);
	SKicadSchemBusTap*		GetTap(size uiIndex);
	size					NumMembers(void);
	char*					GetMember(size uiIndex);
	size					GetMemberIndex(char* szMember);
	bool					Contains(SKicadSchemPoint* psPoint, bool* pbVertical);
};


bool	ParseKicadSchemBusMembers(const char* szBusName, CArrayChars* paszMembers);


#endif // __KICAD_SCHEM_BUS_H__


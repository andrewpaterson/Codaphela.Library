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
#include <math.h>
#include "BaseLib/StringHelper.h"
#include "KicadSchemBus.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemBus::Init(void)
{
	mcSubnet.Init();
	mapcLabels.Init();
	masTaps.Init();
	maszMembers.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemBus::Kill(void)
{
	maszMembers.Kill();
	masTaps.Kill();
	mapcLabels.Kill();
	mcSubnet.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemBus::AddWire(CKicadSchemWire* pcWire)
{
	mcSubnet.AddWire(pcWire);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemBus::AddJunction(CKicadSchemJunction* pcJunction)
{
	mcSubnet.AddJunction(pcJunction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemBus::AddLabel(CKicadSchemLabel* pcLabel)
{
	mapcLabels.Add(pcLabel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemBus::AddTap(CKicadSchemBusEntry* pcEntry, SKicadSchemPoint* psBusPoint, SKicadSchemPoint* psWirePoint, CKicadSchemSubnet* pcMember, bool bVertical)
{
	SKicadSchemBusTap*	psTap;

	psTap = masTaps.Add();
	psTap->pcEntry = pcEntry;
	psTap->sBusPoint = *psBusPoint;
	psTap->sWirePoint = *psWirePoint;
	psTap->pcMember = pcMember;
	psTap->bVertical = bVertical;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the member if the bus doesn't have it.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemBus::AddMember(char* szMember)
{
	if (GetMemberIndex(szMember) == ARRAY_ELEMENT_NOT_FOUND)
	{
		maszMembers.Add(szMember);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The bus's first label, or NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
char* CKicadSchemBus::GetName(void)
{
	if (mapcLabels.NumElements() == 0)
	{
		return NULL;
	}
	return mapcLabels.GetPtr(0)->GetText();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemBus::GetMemberIndex(char* szMember)
{
	return maszMembers.GetIndex(szMember);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the point is on one of the bus's wires, and if it is whether
// that wire runs up and down.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemBus::Contains(SKicadSchemPoint* psPoint, bool* pbVertical)
{
	size				i;
	CKicadSchemWire*	pcWire;

	for (i = 0; i < mcSubnet.NumWires(); i++)
	{
		pcWire = mcSubnet.GetWire(i);
		if (pcWire->Contains(psPoint))
		{
			*pbVertical = fabs(pcWire->GetEnd()->fY - pcWire->GetStart()->fY) > fabs(pcWire->GetEnd()->fX - pcWire->GetStart()->fX);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKicadSchemSubnet* CKicadSchemBus::GetSubnet(void) { return &mcSubnet; }
size CKicadSchemBus::NumLabels(void) { return mapcLabels.NumElements(); }
CKicadSchemLabel* CKicadSchemBus::GetLabel(size uiIndex) { return mapcLabels.SafeGetPtr(uiIndex); }
size CKicadSchemBus::NumTaps(void) { return masTaps.NumElements(); }
SKicadSchemBusTap* CKicadSchemBus::GetTap(size uiIndex) { return masTaps.SafeGet(uiIndex); }
size CKicadSchemBus::NumMembers(void) { return maszMembers.NumElements(); }
char* CKicadSchemBus::GetMember(size uiIndex) { return maszMembers.GetText(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// The members of a vector bus, KiCad's "PREFIX[first..last]": PREFIX
// followed by each number from first to last (either way round).  False
// (and none added) if the name isn't one.
//
//////////////////////////////////////////////////////////////////////////
bool ParseKicadSchemBusMembers(const char* szBusName, CArrayChars* paszMembers)
{
	const char*	szOpen;
	const char*	szDots;
	const char*	szClose;
	const char*	sz;
	int			iFirst;
	int			iLast;
	int			i;
	int			iStep;
	CChars		szMember;

	if (szBusName == NULL)
	{
		return false;
	}
	szOpen = strrchr(szBusName, '[');
	if ((szOpen == NULL) || (szOpen == szBusName))
	{
		return false;
	}
	szDots = strstr(szOpen, "..");
	szClose = strchr(szOpen, ']');
	if ((szDots == NULL) || (szClose == NULL) || (szDots > szClose) || (szClose[1] != '\0'))
	{
		return false;
	}
	for (sz = szOpen + 1; sz < szDots; sz++)
	{
		if ((*sz < '0') || (*sz > '9'))
		{
			return false;
		}
	}
	for (sz = szDots + 2; sz < szClose; sz++)
	{
		if ((*sz < '0') || (*sz > '9'))
		{
			return false;
		}
	}
	if ((szDots == szOpen + 1) || (szClose == szDots + 2))
	{
		return false;
	}

	iFirst = atoi(szOpen + 1);
	iLast = atoi(szDots + 2);
	iStep = (iLast >= iFirst) ? 1 : -1;
	for (i = iFirst;; i += iStep)
	{
		szMember.Init(szBusName, 0, szOpen - szBusName);
		szMember.Append((int32)i);
		paszMembers->Add(szMember.Text());
		szMember.Kill();
		if (i == iLast)
		{
			break;
		}
	}
	return true;
}


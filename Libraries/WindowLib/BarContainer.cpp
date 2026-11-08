/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "ClearCanvasDraw.h"
#include "Canvas.h"
#include "BarContainer.h"


//////////////////////////////////////////////////////////////////////////
//
// An empty bar: iThickness across (a grid has no size of its own), as
// long as it is laid out.  iEndGap pixels stay after the last component
// appended once there is a stretch.
//
//////////////////////////////////////////////////////////////////////////
void CBarContainer::Init(Ptr<CWindow> pWindow, bool bVertical, int32 iThickness, int32 iEndGap)
{
	PreInit();

	CGridContainer::Init(pWindow);
	mbVertical = bVertical;
	mbStretched = false;
	miNextCell = 0;
	miEndGap = iEndGap;
	SetAlignment(CSH_Stretch, CSV_Stretch);
	SetDesiredSize(bVertical ? iThickness : -1, bVertical ? -1 : iThickness);
	mpEnd = OMalloc<CFillContainer>(pWindow);
	mpEnd->SetDesiredSize(bVertical ? 1 : iEndGap, bVertical ? iEndGap : 1);
	PlaceEnd();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBarContainer::Class(void)
{
	CGridContainer::Class();
	M_Pointer(mpEnd);
	U_Int32(miNextCell);
	U_Int32(miEndGap);
	U_Bool(mbVertical);
	U_Bool(mbStretched);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBarContainer::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBarContainer::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// pComponent goes after the last, at its own size, centred across the
// bar unless it is aligned otherwise.
//
//////////////////////////////////////////////////////////////////////////
void CBarContainer::Append(Ptr<CComponent> pComponent)
{
	AddAt(pComponent, miNextCell, 0);
	miNextCell++;
	PlaceEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBarContainer::AddGap(int32 iPixels)
{
	Ptr<CFillContainer>	pGap;

	pGap = OMalloc<CFillContainer>(mpWindow);
	pGap->SetDesiredSize(mbVertical ? 1 : iPixels, mbVertical ? iPixels : 1);
	Append(pGap);
}


//////////////////////////////////////////////////////////////////////////
//
// The space left over goes here: what is appended after it goes to the
// bar's far end.  A bar has one stretch at most; another is ignored.
//
//////////////////////////////////////////////////////////////////////////
void CBarContainer::AddStretch(void)
{
	if (!mbStretched)
	{
		AppendStretch(OMalloc<CFillContainer>(mpWindow));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pComponent is the stretch: it takes the space left over.  False if the
// bar has one already.
//
//////////////////////////////////////////////////////////////////////////
bool CBarContainer::AppendStretch(Ptr<CComponent> pComponent)
{
	if (mbStretched)
	{
		return false;
	}
	mbStretched = true;
	AddAt(pComponent, miNextCell, 1);
	miNextCell++;
	PlaceEnd();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A line iLength across the bar, centred in iSpace along it.
//
//////////////////////////////////////////////////////////////////////////
void CBarContainer::AppendLine(int32 iSpace, int32 iLength, ARGB32 sColour)
{
	Ptr<CFillContainer>	pSpace;
	Ptr<CCanvas>		pLine;

	pLine = OMalloc<CCanvas>(mpWindow, OMalloc<CClearCanvasDraw>(sColour));
	pLine->SetDesiredSize(mbVertical ? iLength : 1, mbVertical ? 1 : iLength);
	pLine->SetAlignment(CSH_Center, CSV_Center);
	pSpace = OMalloc<CFillContainer>(mpWindow);
	pSpace->SetDesiredSize(mbVertical ? iLength : iSpace, mbVertical ? iSpace : iLength);
	pSpace->AddComponent(pLine);
	Append(pSpace);
}


//////////////////////////////////////////////////////////////////////////
//
// The end goes after the last component: taking the space left over
// until there is a stretch, then only the end gap.
//
//////////////////////////////////////////////////////////////////////////
void CBarContainer::PlaceEnd(void)
{
	RemoveComponent(mpEnd);
	AddAt(mpEnd, miNextCell, mbStretched ? 0 : 1);
}


//////////////////////////////////////////////////////////////////////////
//
// pComponent in the bar's iCell, filling it across the bar, with
// iLengthWeight of the space left over along it.
//
//////////////////////////////////////////////////////////////////////////
void CBarContainer::AddAt(Ptr<CComponent> pComponent, int32 iCell, int iLengthWeight)
{
	if (mbVertical)
	{
		AddComponent(pComponent, 0, iCell, 1, 1, 1, iLengthWeight);
	}
	else
	{
		AddComponent(pComponent, iCell, 0, 1, 1, iLengthWeight, 1);
	}
}


bool CBarContainer::IsVertical(void) { return mbVertical; }
bool CBarContainer::IsStretched(void) { return mbStretched; }


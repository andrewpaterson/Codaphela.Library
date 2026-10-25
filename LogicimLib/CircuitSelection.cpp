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
#include "CircuitPainter.h"
#include "CircuitView.h"
#include "CircuitSelection.h"


//A selection marker is three pixels from its corner at zoom one, as Logicim's paintSelectionRectangle.
#define SELECTION_MARKER_RADIUS		0.3


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSelection::Init(void)
{
	mapcComponents.Init();
	mapcTraces.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSelection::Kill(void)
{
	mapcTraces.Kill();
	mapcComponents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSelection::Clear(void)
{
	mapcComponents.ReInit();
	mapcTraces.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitSelection::IsEmpty(void)
{
	return (mapcComponents.NumElements() == 0) && (mapcTraces.NumElements() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSelection::Set(CCircuitSelection* pcSource)
{
	size	i;

	Clear();
	for (i = 0; i < pcSource->mapcComponents.NumElements(); i++)
	{
		mapcComponents.Add(pcSource->mapcComponents.GetPtr(i));
	}
	for (i = 0; i < pcSource->mapcTraces.NumElements(); i++)
	{
		mapcTraces.Add(pcSource->mapcTraces.GetPtr(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The views a selection rectangle picked combined with the selection
// before it, as Logicim's calculateSelection.  Added views go after those
// already selected.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSelection::Set(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, CCircuitSelection* pcPrevious, ESelectionMode eMode)
{
	size	i;

	if (eMode == SM_Replace)
	{
		Clear();
	}
	else
	{
		Set(pcPrevious);
	}

	for (i = 0; i < papcComponents->NumElements(); i++)
	{
		if (eMode == SM_Subtract)
		{
			Remove(papcComponents->GetPtr(i));
		}
		else
		{
			Add(papcComponents->GetPtr(i));
		}
	}
	for (i = 0; i < papcTraces->NumElements(); i++)
	{
		if (eMode == SM_Subtract)
		{
			Remove(papcTraces->GetPtr(i));
		}
		else
		{
			Add(papcTraces->GetPtr(i));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Views already selected aren't added again.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSelection::Add(CComponentView* pcComponent)
{
	if (!Contains(pcComponent))
	{
		mapcComponents.Add(pcComponent);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSelection::Add(CTraceView* pcTrace)
{
	if (!Contains(pcTrace))
	{
		mapcTraces.Add(pcTrace);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSelection::Remove(CComponentView* pcComponent)
{
	size	uiIndex;

	uiIndex = mapcComponents.Find(pcComponent);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		mapcComponents.RemoveAt(uiIndex, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSelection::Remove(CTraceView* pcTrace)
{
	size	uiIndex;

	uiIndex = mapcTraces.Find(pcTrace);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		mapcTraces.RemoveAt(uiIndex, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitSelection::Contains(CComponentView* pcComponent)
{
	return mapcComponents.Find(pcComponent) != ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitSelection::Contains(CTraceView* pcTrace)
{
	return mapcTraces.Find(pcTrace) != ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// The same views in any order, as Logicim's hasSelectionChanged.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitSelection::Equals(CCircuitSelection* pcOther)
{
	size	i;

	if ((mapcComponents.NumElements() != pcOther->mapcComponents.NumElements()) || (mapcTraces.NumElements() != pcOther->mapcTraces.NumElements()))
	{
		return false;
	}
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		if (!pcOther->Contains(mapcComponents.GetPtr(i)))
		{
			return false;
		}
	}
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		if (!pcOther->Contains(mapcTraces.GetPtr(i)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircuitSelection::NumComponents(void) { return mapcComponents.NumElements(); }
CComponentView* CCircuitSelection::GetComponent(size uiIndex) { return mapcComponents.SafeGetPtr(uiIndex); }
CArrayComponentViewPtr* CCircuitSelection::GetComponents(void) { return &mapcComponents; }
size CCircuitSelection::NumTraces(void) { return mapcTraces.NumElements(); }
CTraceView* CCircuitSelection::GetTrace(size uiIndex) { return mapcTraces.SafeGetPtr(uiIndex); }
CArrayTraceViewPtr* CCircuitSelection::GetTraces(void) { return &mapcTraces; }


//////////////////////////////////////////////////////////////////////////
//
// The one selected component, as Logicim's getSingleSelectionStaticView.
// NULL if none or more than one are selected.
//
//////////////////////////////////////////////////////////////////////////
CComponentView* CCircuitSelection::GetSingleComponent(void)
{
	if (mapcComponents.NumElements() == 1)
	{
		return mapcComponents.GetPtr(0);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The average of the components' positions and the wires' ends, rounded,
// as Logicim's getViewsCenter.  False if nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitSelection::GetCentre(SInt32Vec2* psCentre)
{
	size			i;
	float64			fX;
	float64			fY;
	size			uiCount;
	SInt32Vec2		sPosition;
	CTraceView*		pcTrace;

	if (IsEmpty())
	{
		return false;
	}

	fX = 0;
	fY = 0;
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		sPosition = mapcComponents.GetPtr(i)->GetPosition();
		fX += sPosition.x;
		fY += sPosition.y;
	}
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		fX += pcTrace->GetStart().x + pcTrace->GetEnd().x;
		fY += pcTrace->GetStart().y + pcTrace->GetEnd().y;
	}
	uiCount = mapcComponents.NumElements() + mapcTraces.NumElements() * 2;
	psCentre->Init(RoundToGrid(fX / uiCount), RoundToGrid(fY / uiCount));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the grid position is in a selected component's box or on a
// selected wire, as Logicim's isInSelectedComponent.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitSelection::IsOn(SFloat64Vec2 sPosition)
{
	size			i;
	SFloat64Vec2	sMinimum;
	SFloat64Vec2	sMaximum;
	SInt32Vec2		sPoint;

	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		if (mapcComponents.GetPtr(i)->GetBoundingBox(&sMinimum, &sMaximum) && IsInBox(sPosition, sMinimum, sMaximum))
		{
			return true;
		}
	}
	sPoint.Init(RoundToGrid(sPosition.x), RoundToGrid(sPosition.y));
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		if (mapcTraces.GetPtr(i)->Contains(sPoint))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A marker on each corner of each selected component's box and on each
// end of each selected wire, as Logicim's paintSelected.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitSelection::Paint(CCircuitPainter* pcPainter)
{
	size			i;
	CTraceView*		pcTrace;
	ARGB32			sColour;

	sColour = pcPainter->GetColours()->msSelected;
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		PaintSelectionMarkers(pcPainter, mapcComponents.GetPtr(i), sColour);
	}
	pcPainter->ClearTransform();
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		PaintSelectionMarker(pcPainter, pcTrace->GetStart().x, pcTrace->GetStart().y, sColour);
		PaintSelectionMarker(pcPainter, pcTrace->GetEnd().x, pcTrace->GetEnd().y, sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Shift selects more, control and shift selects less, as Logicim's
// calculateSelectionMode.
//
//////////////////////////////////////////////////////////////////////////
ESelectionMode CalculateSelectionMode(bool bControl, bool bShift)
{
	if (bControl && bShift)
	{
		return SM_Subtract;
	}
	else if (bShift)
	{
		return SM_Add;
	}
	return SM_Replace;
}


//////////////////////////////////////////////////////////////////////////
//
// A marker on each corner of the component's box; the selection's colour
// for a selected component, the hover colour for the one under the
// pointer.
//
//////////////////////////////////////////////////////////////////////////
void PaintSelectionMarkers(CCircuitPainter* pcPainter, CComponentView* pcComponent, ARGB32 sColour)
{
	SFloat64Vec2	sMinimum;
	SFloat64Vec2	sMaximum;

	if (pcComponent->GetBoundingBox(&sMinimum, &sMaximum))
	{
		pcPainter->ClearTransform();
		PaintSelectionMarker(pcPainter, sMinimum.x, sMinimum.y, sColour);
		PaintSelectionMarker(pcPainter, sMaximum.x, sMinimum.y, sColour);
		PaintSelectionMarker(pcPainter, sMinimum.x, sMaximum.y, sColour);
		PaintSelectionMarker(pcPainter, sMaximum.x, sMaximum.y, sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A small filled square, bordered, centred on the grid position.  The
// painter must have no transform.
//
//////////////////////////////////////////////////////////////////////////
void PaintSelectionMarker(CCircuitPainter* pcPainter, float64 x, float64 y, ARGB32 sColour)
{
	SFloat64Vec2	asPoints[4];

	asPoints[0].Init(x - SELECTION_MARKER_RADIUS, y - SELECTION_MARKER_RADIUS);
	asPoints[1].Init(x + SELECTION_MARKER_RADIUS, y - SELECTION_MARKER_RADIUS);
	asPoints[2].Init(x + SELECTION_MARKER_RADIUS, y + SELECTION_MARKER_RADIUS);
	asPoints[3].Init(x - SELECTION_MARKER_RADIUS, y + SELECTION_MARKER_RADIUS);
	pcPainter->Polygon(asPoints, 4, true, true, 1, sColour, pcPainter->GetColours()->msShapeBorder);
}


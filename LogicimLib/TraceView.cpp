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
#include <math.h>
#include "BaseLib/Logger.h"
#include "ComponentShapes.h"
#include "ViewPath.h"
#include "TraceView.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTraceView::Init(int32 x0, int32 y0, int32 x1, int32 y1)
{
	msStart.Init(x0, y0);
	msEnd.Init(x1, y1);
	masTraces.Init();
	mbStartJunction = false;
	mbEndJunction = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Initialises the wire from a <Trace x0 y0 x1 y1/> element.  On failure
// the wire is left uninitialised.
//
//////////////////////////////////////////////////////////////////////////
bool CTraceView::LoadXML(CMarkupTag* pcTag)
{
	int32	x0;
	int32	y0;
	int32	x1;
	int32	y1;

	if (!pcTag->GetInt32Attribute("x0", &x0) || !pcTag->GetInt32Attribute("y0", &y0) || !pcTag->GetInt32Attribute("x1", &x1) || !pcTag->GetInt32Attribute("y1", &y1))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs whole number x0, y0, x1 and y1 attributes.", NULL);
	}
	Init(x0, y0, x1, y1);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTraceView::SaveXML(CXMLWriter* pcWriter)
{
	pcWriter->Begin("Trace");
	pcWriter->Attribute("x0", msStart.x);
	pcWriter->Attribute("y0", msStart.y);
	pcWriter->Attribute("x1", msEnd.x);
	pcWriter->Attribute("y1", msEnd.y);
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// The traces belong to the circuit.
//
//////////////////////////////////////////////////////////////////////////
void CTraceView::Kill(void)
{
	size	i;

	for (i = 0; i < masTraces.NumElements(); i++)
	{
		masTraces.Get(i)->apcTraces.Kill();
	}
	masTraces.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTraceView::Paint(CCircuitPainter* pcPainter)
{
	ARGB32		sColour;
	float64		fRadius;

	sColour = GetColour(pcPainter);
	pcPainter->Line(msStart.x, msStart.y, msEnd.x, msEnd.y, SHAPE_LINE_WIDTH, sColour);

	fRadius = pcPainter->GetViewport()->GetConnectionRadius();
	if (mbStartJunction)
	{
		pcPainter->Dot(msStart.x, msStart.y, fRadius, sColour);
	}
	if (mbEndJunction)
	{
		pcPainter->Dot(msEnd.x, msEnd.y, fRadius, sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The average voltage of the traces of the painter's path and circuit
// simulation that are driven.  A trace view with no traces is
// disconnected.
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CTraceView::GetColour(CCircuitPainter* pcPainter)
{
	CCircuitColours*			pcColours;
	size						i;
	size						uiDriven;
	float64						fAverage;
	float32						fVoltage;
	int64						iTime;
	CArrayTraceViewTracePtr*	papcTraces;

	pcColours = pcPainter->GetColours();
	iTime = pcPainter->GetTime();
	papcTraces = GetTraces(pcPainter->GetViewPath(), pcPainter->GetCircuitSimulation());
	if ((papcTraces == NULL) || (papcTraces->NumElements() == 0) || (iTime == -1))
	{
		return pcColours->msDisconnectedTrace;
	}
	if (papcTraces->NumElements() == 1)
	{
		return pcColours->GetColourForTrace(papcTraces->GetPtr(0), iTime);
	}

	fAverage = 0;
	uiDriven = 0;
	for (i = 0; i < papcTraces->NumElements(); i++)
	{
		fVoltage = papcTraces->GetPtr(i)->GetVoltage(iTime);
		if (!isnan(fVoltage))
		{
			fAverage += fVoltage;
			uiDriven++;
		}
	}
	if (uiDriven == 0)
	{
		return pcColours->msTraceUndriven;
	}
	return pcColours->GetColourForVoltage((float32)(fAverage / (float64)uiDriven));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the point is on the wire, ends included.  Wires that aren't
// horizontal or vertical only contain their ends.
//
//////////////////////////////////////////////////////////////////////////
bool CTraceView::Contains(SInt32Vec2 sPoint)
{
	int32	iMinimum;
	int32	iMaximum;

	if (IsEnd(sPoint))
	{
		return true;
	}
	if ((msStart.x == msEnd.x) && (sPoint.x == msStart.x))
	{
		iMinimum = (msStart.y < msEnd.y) ? msStart.y : msEnd.y;
		iMaximum = (msStart.y > msEnd.y) ? msStart.y : msEnd.y;
		return (sPoint.y >= iMinimum) && (sPoint.y <= iMaximum);
	}
	if ((msStart.y == msEnd.y) && (sPoint.y == msStart.y))
	{
		iMinimum = (msStart.x < msEnd.x) ? msStart.x : msEnd.x;
		iMaximum = (msStart.x > msEnd.x) ? msStart.x : msEnd.x;
		return (sPoint.x >= iMinimum) && (sPoint.x <= iMaximum);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTraceView::IsEnd(SInt32Vec2 sPoint)
{
	return (sPoint == msStart) || (sPoint == msEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// Wires join where an end of one is on the other.  Wires that cross
// without either ending there don't.
//
//////////////////////////////////////////////////////////////////////////
bool CTraceView::Touches(CTraceView* pcOther)
{
	return Contains(pcOther->msStart) || Contains(pcOther->msEnd) || pcOther->Contains(msStart) || pcOther->Contains(msEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTraceView::SetTraces(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CArrayTraceViewTracePtr* papcTraces)
{
	size					i;
	SViewPathTraces*		psTraces;

	ClearTraces(pcViewPath, pcCircuitSimulation);
	psTraces = masTraces.Add();
	psTraces->pcViewPath = pcViewPath;
	psTraces->pcCircuitSimulation = pcCircuitSimulation;
	psTraces->apcTraces.Init();
	for (i = 0; i < papcTraces->NumElements(); i++)
	{
		psTraces->apcTraces.Add(papcTraces->GetPtr(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTraceView::ClearTraces(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	SViewPathTraces*	psTraces;
	size				uiIndex;

	psTraces = FindTraces(pcViewPath, pcCircuitSimulation, &uiIndex);
	if (psTraces != NULL)
	{
		psTraces->apcTraces.Kill();
		masTraces.RemoveAt(uiIndex, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the wire has no traces for the path and circuit simulation.
//
//////////////////////////////////////////////////////////////////////////
CArrayTraceViewTracePtr* CTraceView::GetTraces(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	SViewPathTraces*	psTraces;

	psTraces = FindTraces(pcViewPath, pcCircuitSimulation, NULL);
	if (psTraces == NULL)
	{
		return NULL;
	}
	return &psTraces->apcTraces;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTraceView::NumTraces(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	CArrayTraceViewTracePtr*	papcTraces;

	papcTraces = GetTraces(pcViewPath, pcCircuitSimulation);
	if (papcTraces == NULL)
	{
		return 0;
	}
	return papcTraces->NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTrace* CTraceView::GetTrace(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, size uiIndex)
{
	CArrayTraceViewTracePtr*	papcTraces;

	papcTraces = GetTraces(pcViewPath, pcCircuitSimulation);
	if (papcTraces == NULL)
	{
		return NULL;
	}
	return papcTraces->SafeGetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SViewPathTraces* CTraceView::FindTraces(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, size* puiIndex)
{
	return FindViewPathSimulation(&masTraces, pcViewPath, pcCircuitSimulation, puiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTraceView::SetJunctions(bool bStart, bool bEnd) { mbStartJunction = bStart; mbEndJunction = bEnd; }
bool CTraceView::IsStartJunction(void) { return mbStartJunction; }
bool CTraceView::IsEndJunction(void) { return mbEndJunction; }
SInt32Vec2 CTraceView::GetStart(void) { return msStart; }
SInt32Vec2 CTraceView::GetEnd(void) { return msEnd; }


//////////////////////////////////////////////////////////////////////////
//
// Moves the wire's ends; a simulated wire must be rewired after.
//
//////////////////////////////////////////////////////////////////////////
void CTraceView::SetEnds(SInt32Vec2 sStart, SInt32Vec2 sEnd)
{
	msStart = sStart;
	msEnd = sEnd;
}


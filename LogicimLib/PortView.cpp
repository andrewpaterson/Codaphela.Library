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
#include "BaseLib/Logger.h"
#include "MicroprocessorLib/CircuitComponent.h"
#include "ComponentShapes.h"
#include "ComponentView.h"
#include "PortView.h"


//////////////////////////////////////////////////////////////////////////
//
// The position is in the owner's grid space, facing North.
//
//////////////////////////////////////////////////////////////////////////
void CPortView::Init(CComponentView* pcOwner, const char* szPortName, int32 x, int32 y)
{
	CArrayChars		aszNames;

	aszNames.Init();
	aszNames.Add(szPortName);
	Init(pcOwner, &aszNames, x, y);
	aszNames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortView::Init(CComponentView* pcOwner, CArrayChars* paszPortNames, int32 x, int32 y)
{
	mpcOwner = pcOwner;
	msRelativePosition.Init(x, y);
	maszPortNames.Init(paszPortNames);
	mszText.Init();
	mbInverting = false;
	msBubbleCentre.Init(0, 0);
	mfBubbleDiameter = PORT_BUBBLE_DIAMETER;
	mbClock = false;
	masClockNotch[0].Init(0, 0);
	masClockNotch[1].Init(0, 0);
	masClockNotch[2].Init(0, 0);
	mbDrawn = true;
	masPorts.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortView::Kill(void)
{
	size	i;

	for (i = 0; i < masPorts.NumElements(); i++)
	{
		masPorts.Get(i)->apcPorts.Kill();
	}
	masPorts.Kill();
	mszText.Kill();
	maszPortNames.Kill();
	mpcOwner = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts a bubble where the port was and moves the port a grid unit further
// out, in the direction the port faces.
//
//////////////////////////////////////////////////////////////////////////
void CPortView::SetInverting(ERotation eFacing)
{
	SInt32Vec2		sPortOffset;
	SFloat64Vec2	sBubbleOffset;

	mbInverting = true;
	sPortOffset.Init(0, -1);
	RotatePoint(eFacing, &sPortOffset);
	sBubbleOffset.Init(0, -0.5);
	RotatePoint(eFacing, &sBubbleOffset);

	msBubbleCentre.Init(msRelativePosition.x + sBubbleOffset.x, msRelativePosition.y + sBubbleOffset.y);
	msRelativePosition.x += sPortOffset.x;
	msRelativePosition.y += sPortOffset.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortView::SetDrawn(bool bDrawn)
{
	mbDrawn = bDrawn;
}


//////////////////////////////////////////////////////////////////////////
//
// The port is on the component's edge and faces eFacing out of it (as
// SetInverting).  The notch is a grid unit along the edge and points half
// a grid unit in.  Call before SetInverting, which moves the port.
//
//////////////////////////////////////////////////////////////////////////
void CPortView::SetClock(ERotation eFacing)
{
	size	i;

	mbClock = true;
	masClockNotch[0].Init(-0.5, 0);
	masClockNotch[1].Init(0, 0.5);
	masClockNotch[2].Init(0.5, 0);
	for (i = 0; i < 3; i++)
	{
		RotatePoint(eFacing, &masClockNotch[i]);
		masClockNotch[i].x += msRelativePosition.x;
		masClockNotch[i].y += msRelativePosition.y;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortView::SetText(const char* szText)
{
	mszText.Set(szText);
}


//////////////////////////////////////////////////////////////////////////
//
// Turns the port's position (and bubble) about the owner, for components
// drawn in another direction to their ports.
//
//////////////////////////////////////////////////////////////////////////
void CPortView::Rotate(ERotation eRotation)
{
	RotatePoint(eRotation, &msRelativePosition);
	RotatePoint(eRotation, &msBubbleCentre);
}


//////////////////////////////////////////////////////////////////////////
//
// Finds the component's ports by name for the path and circuit
// simulation.  Fails (and holds none for them) if one is missing.
//
//////////////////////////////////////////////////////////////////////////
bool CPortView::Connect(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CCircuitComponent* pcComponent)
{
	size				i;
	CPort*				pcPort;
	char*				szName;
	SViewPathPorts*		psPorts;

	Disconnect(pcViewPath, pcCircuitSimulation);
	psPorts = masPorts.Add();
	psPorts->pcViewPath = pcViewPath;
	psPorts->pcCircuitSimulation = pcCircuitSimulation;
	psPorts->apcPorts.Init();
	for (i = 0; i < maszPortNames.NumElements(); i++)
	{
		szName = maszPortNames.GetText(i);
		pcPort = pcComponent->GetPort(szName);
		if (pcPort == NULL)
		{
			Disconnect(pcViewPath, pcCircuitSimulation);
			return gcLogger.Error2(__METHOD__, " Cannot find port [", szName, "] on [", pcComponent->GetName(), "].", NULL);
		}
		psPorts->apcPorts.Add(pcPort);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortView::Disconnect(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	SViewPathPorts*		psPorts;
	size				uiIndex;

	psPorts = FindPorts(pcViewPath, pcCircuitSimulation, &uiIndex);
	if (psPorts != NULL)
	{
		psPorts->apcPorts.Kill();
		masPorts.RemoveAt(uiIndex, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SViewPathPorts* CPortView::FindPorts(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, size* puiIndex)
{
	return FindViewPathSimulation(&masPorts, pcViewPath, pcCircuitSimulation, puiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the port view has no ports for the path and circuit simulation.
//
//////////////////////////////////////////////////////////////////////////
CArrayPortPtr* CPortView::GetPorts(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	SViewPathPorts*		psPorts;

	psPorts = FindPorts(pcViewPath, pcCircuitSimulation, NULL);
	if (psPorts == NULL)
	{
		return NULL;
	}
	return &psPorts->apcPorts;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPortView::NumPorts(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	CArrayPortPtr*	papcPorts;

	papcPorts = GetPorts(pcViewPath, pcCircuitSimulation);
	if (papcPorts == NULL)
	{
		return 0;
	}
	return papcPorts->NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPort* CPortView::GetPort(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, size uiIndex)
{
	CArrayPortPtr*	papcPorts;

	papcPorts = GetPorts(pcViewPath, pcCircuitSimulation);
	if (papcPorts == NULL)
	{
		return NULL;
	}
	return papcPorts->SafeGetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// Expects the painter to hold the owner's transform.  The dot is coloured
// by the ports of the painter's path and circuit simulation.
//
//////////////////////////////////////////////////////////////////////////
void CPortView::Paint(CCircuitPainter* pcPainter)
{
	CCircuitColours*	pcColours;
	ARGB32				sColour;
	CArrayPortPtr*		papcPorts;

	if (!mbDrawn)
	{
		return;
	}
	pcColours = pcPainter->GetColours();
	if (mbClock)
	{
		pcPainter->Polygon(masClockNotch, 3, false, true, PORT_CLOCK_NOTCH_WIDTH, 0, pcColours->msShapeBorder);
	}
	if (mbInverting)
	{
		pcPainter->Circle(msBubbleCentre.x, msBubbleCentre.y, mfBubbleDiameter / 2, false, true, SHAPE_LINE_WIDTH, 0, pcColours->msShapeBorder);
	}

	papcPorts = GetPorts(pcPainter->GetViewPath(), pcPainter->GetCircuitSimulation());
	if ((papcPorts != NULL) && (papcPorts->NumElements() > 0))
	{
		sColour = pcColours->GetColourForPorts(papcPorts, pcPainter->GetTime());
	}
	else
	{
		sColour = pcColours->msDisconnectedTrace;
	}
	pcPainter->Dot(msRelativePosition.x, msRelativePosition.y, pcPainter->GetViewport()->GetConnectionRadius(), sColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPortView::IncludeBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds)
{
	float64		fRadius;

	if (!bHasBounds)
	{
		psMinimum->Init(msRelativePosition.x, msRelativePosition.y);
		psMaximum->Init(msRelativePosition.x, msRelativePosition.y);
	}
	IncludeBoundsPoint(psMinimum, psMaximum, msRelativePosition.x, msRelativePosition.y);
	if (mbInverting)
	{
		fRadius = mfBubbleDiameter / 2;
		IncludeBoundsPoint(psMinimum, psMaximum, msBubbleCentre.x - fRadius, msBubbleCentre.y - fRadius);
		IncludeBoundsPoint(psMinimum, psMaximum, msBubbleCentre.x + fRadius, msBubbleCentre.y + fRadius);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// In the owner's grid space, facing North.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CPortView::GetRelativePosition(void)
{
	return msRelativePosition;
}


//////////////////////////////////////////////////////////////////////////
//
// In the circuit's grid space.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CPortView::GetGridPosition(void)
{
	SInt32Vec2	sPosition;
	SInt32Vec2	sOwner;

	sPosition = msRelativePosition;
	RotatePoint(mpcOwner->GetRotation(), &sPosition);
	sOwner = mpcOwner->GetPosition();
	sPosition.x += sOwner.x;
	sPosition.y += sOwner.y;
	return sPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPortView::IsInverting(void) { return mbInverting; }
bool CPortView::IsClock(void) { return mbClock; }
SFloat64Vec2* CPortView::GetClockNotch(void) { return masClockNotch; }
char* CPortView::GetText(void) { return mszText.Text(); }
size CPortView::NumPortNames(void) { return maszPortNames.NumElements(); }
char* CPortView::GetPortName(size uiIndex) { return maszPortNames.GetText(uiIndex); }
CComponentView* CPortView::GetOwner(void) { return mpcOwner; }


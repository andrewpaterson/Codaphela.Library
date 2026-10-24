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
	mapcPorts.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortView::Kill(void)
{
	mapcPorts.Kill();
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
void CPortView::SetClock(bool bClock)
{
	mbClock = bClock;
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
// Finds the component's ports by name.  Fails (and holds none) if one is
// missing.
//
//////////////////////////////////////////////////////////////////////////
bool CPortView::Connect(CCircuitComponent* pcComponent)
{
	size	i;
	CPort*	pcPort;
	char*	szName;

	mapcPorts.ReInit();
	for (i = 0; i < maszPortNames.NumElements(); i++)
	{
		szName = maszPortNames.GetText(i);
		pcPort = pcComponent->GetPort(szName);
		if (pcPort == NULL)
		{
			mapcPorts.ReInit();
			return gcLogger.Error2(__METHOD__, " Cannot find port [", szName, "] on [", pcComponent->GetName(), "].", NULL);
		}
		mapcPorts.Add(pcPort);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortView::Disconnect(void)
{
	mapcPorts.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Expects the painter to hold the owner's transform.
//
//////////////////////////////////////////////////////////////////////////
void CPortView::Paint(CCircuitPainter* pcPainter)
{
	CCircuitColours*	pcColours;
	ARGB32				sColour;

	pcColours = pcPainter->GetColours();
	if (mbInverting)
	{
		pcPainter->Circle(msBubbleCentre.x, msBubbleCentre.y, mfBubbleDiameter / 2, false, true, SHAPE_LINE_WIDTH, 0, pcColours->msShapeBorder);
	}

	if (mapcPorts.NumElements() > 0)
	{
		sColour = pcColours->GetColourForPorts(&mapcPorts, pcPainter->GetTime());
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
char* CPortView::GetText(void) { return mszText.Text(); }
size CPortView::NumPortNames(void) { return maszPortNames.NumElements(); }
char* CPortView::GetPortName(size uiIndex) { return maszPortNames.GetText(uiIndex); }
size CPortView::NumPorts(void) { return mapcPorts.NumElements(); }
CPort* CPortView::GetPort(size uiIndex) { return mapcPorts.SafeGetPtr(uiIndex); }
CArrayPortPtr* CPortView::GetPorts(void) { return &mapcPorts; }
CComponentView* CPortView::GetOwner(void) { return mpcOwner; }


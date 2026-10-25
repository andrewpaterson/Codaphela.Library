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
#include <stdlib.h>
#include "BaseLib/ConstructorCall.h"
#include "CircuitPainter.h"
#include "CircuitEditor.h"
#include "WirePullEdit.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStartEditInPort::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStartEditInPort::Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Dragging pulls a wire.
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CStartEditInPort::Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	CWirePullEdit*	pcPull;

	pcPull = NewMalloc<CWirePullEdit>();
	pcPull->Init();
	return pcPull;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CStartEditInPort::Rotate(bool bRight, CCircuitEdit* pcEdit)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
// A click on a port or a wire selects as a click anywhere does.
//
//////////////////////////////////////////////////////////////////////////
void CStartEditInPort::Done(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	CCircuitSelection	cPrevious;

	cPrevious.Init();
	cPrevious.Set(pcEdit->GetEditor()->GetSelection());
	pcEdit->GetEditor()->SelectInRectangle(sPosition, sPosition, &cPrevious);
	cPrevious.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStartEditInPort::Discard(CCircuitEdit* pcEdit)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CStartEditInPort::CanTransformComponents(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWirePullEdit::Init(void)
{
	msFirst.Init(0, 0);
	mbHasMiddle = false;
	mbFirstHorizontal = false;
	msMiddle.Init(0, 0);
	mbHasSecond = false;
	msSecond.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The wire starts on the grid point the edit started at.
//
//////////////////////////////////////////////////////////////////////////
void CWirePullEdit::Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	msFirst.Init(RoundToGrid(sPosition.x), RoundToGrid(sPosition.y));
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing stays selected once a wire is being pulled.
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CWirePullEdit::Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	Update(RoundToGrid(sPosition.x), RoundToGrid(sPosition.y));
	if (!IsEmpty())
	{
		pcEdit->GetEditor()->GetSelection()->Clear();
	}
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CWirePullEdit::Rotate(bool bRight, CCircuitEdit* pcEdit)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWirePullEdit::Done(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	if (!IsEmpty())
	{
		pcEdit->GetEditor()->PullWire(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWirePullEdit::Discard(CCircuitEdit* pcEdit)
{
}


//////////////////////////////////////////////////////////////////////////
//
// As a disconnected wire, as Logicim's.
//
//////////////////////////////////////////////////////////////////////////
void CWirePullEdit::Paint(CCircuitPainter* pcPainter)
{
	ARGB32	sColour;

	if (!mbHasMiddle)
	{
		return;
	}
	pcPainter->ClearTransform();
	sColour = pcPainter->GetColours()->msDisconnectedTrace;
	pcPainter->Line(msFirst.x, msFirst.y, msMiddle.x, msMiddle.y, SHAPE_LINE_WIDTH, sColour);
	if (mbHasSecond)
	{
		pcPainter->Line(msMiddle.x, msMiddle.y, msSecond.x, msSecond.y, SHAPE_LINE_WIDTH, sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWirePullEdit::CanTransformComponents(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's WirePull.update.  The pointer straight along the first
// point's row or column starts the wire again that way; off it, the wire
// goes the way it first went as far as the pointer's row or column and
// then turns to the pointer.
//
//////////////////////////////////////////////////////////////////////////
void CWirePullEdit::Update(int32 x, int32 y)
{
	if ((x == msFirst.x) && (y == msFirst.y))
	{
		mbHasMiddle = false;
		mbHasSecond = false;
		return;
	}

	if ((x == msFirst.x) || (y == msFirst.y))
	{
		mbHasMiddle = true;
		mbFirstHorizontal = (y == msFirst.y);
		msMiddle.Init(x, y);
		mbHasSecond = false;
		return;
	}

	if (!mbHasMiddle)
	{
		mbHasMiddle = true;
		mbFirstHorizontal = abs(x - msFirst.x) >= abs(y - msFirst.y);
	}
	if (mbFirstHorizontal)
	{
		msMiddle.Init(x, msFirst.y);
	}
	else
	{
		msMiddle.Init(msFirst.x, y);
	}
	mbHasSecond = true;
	msSecond.Init(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing has been pulled out yet.
//
//////////////////////////////////////////////////////////////////////////
bool CWirePullEdit::IsEmpty(void)
{
	return !mbHasMiddle;
}


//////////////////////////////////////////////////////////////////////////
//
// The wire's one or two lines.
//
//////////////////////////////////////////////////////////////////////////
void CWirePullEdit::GetLines(CArrayTraceLine* pasLines)
{
	STraceLine*	psLine;

	if (mbHasMiddle)
	{
		psLine = pasLines->Add();
		psLine->Init(msFirst, msMiddle);
	}
	if (mbHasSecond)
	{
		psLine = pasLines->Add();
		psLine->Init(msMiddle, msSecond);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CWirePullEdit::GetFirst(void)
{
	return msFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the wire ends, its first point if it's empty.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CWirePullEdit::GetEnd(void)
{
	if (mbHasSecond)
	{
		return msSecond;
	}
	else if (mbHasMiddle)
	{
		return msMiddle;
	}
	return msFirst;
}


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
#include "BaseLib/ConstructorCall.h"
#include "CircuitPainter.h"
#include "CircuitEditor.h"
#include "MoveComponentsEdit.h"
#include "SelectionEdit.h"


//The rectangle's dashes and gaps, in pixels, as Logicim's.
#define SELECTION_DASH_PIXELS		9.0


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSelectionEdit::Init(void)
{
	mpcEditor = NULL;
	mcPrevious.Init();
	msStart.Init(0, 0);
	msEnd.Init(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSelectionEdit::Kill(void)
{
	mcPrevious.Kill();
	mpcEditor = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Remembers the selection the rectangle changes.
//
//////////////////////////////////////////////////////////////////////////
void CSelectionEdit::Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	mpcEditor = pcEdit->GetEditor();
	mcPrevious.Set(mpcEditor->GetSelection());
	msStart = sPosition;
	msEnd = sPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CSelectionEdit::Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	msEnd = sPosition;
	mpcEditor->SelectInRectangle(msStart, msEnd, &mcPrevious);
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
// A selection doesn't turn.
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CSelectionEdit::Transform(CCircuitEdit* pcEdit)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
// The rectangle ends where it was last dragged to.  A changed selection
// can be undone.
//
//////////////////////////////////////////////////////////////////////////
void CSelectionEdit::Done(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	if (mpcEditor->SelectInRectangle(msStart, msEnd, &mcPrevious))
	{
		mpcEditor->PushUndo(true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSelectionEdit::Discard(CCircuitEdit* pcEdit)
{
}


//////////////////////////////////////////////////////////////////////////
//
// A dashed line in grid space, its dashes and gaps fDash long.
//
//////////////////////////////////////////////////////////////////////////
static void PaintDashedLine(CCircuitPainter* pcPainter, float64 x0, float64 y0, float64 x1, float64 y1, float64 fDash, ARGB32 sColour)
{
	float64		fLength;
	float64		fAlong;
	float64		fEnd;
	float64		fDX;
	float64		fDY;

	fLength = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
	if (fLength == 0)
	{
		return;
	}
	fDX = (x1 - x0) / fLength;
	fDY = (y1 - y0) / fLength;
	for (fAlong = 0; fAlong < fLength; fAlong += fDash * 2)
	{
		fEnd = (fAlong + fDash < fLength) ? fAlong + fDash : fLength;
		pcPainter->Line(x0 + fDX * fAlong, y0 + fDY * fAlong, x0 + fDX * fEnd, y0 + fDY * fEnd, 1, sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's SelectionEdit.paint.  The marks by the rectangle's end
// corner are sized in pixels, as Logicim's are.
//
//////////////////////////////////////////////////////////////////////////
void CSelectionEdit::Paint(CCircuitPainter* pcPainter)
{
	float64			fPixel;
	ARGB32			sColour;
	float64			x;
	float64			y;
	float64			fBox;
	ESelectionMode	eMode;

	if (mpcEditor == NULL)
	{
		return;
	}

	pcPainter->ClearTransform();
	fPixel = 1.0 / pcPainter->GetViewport()->GetPixelsPerGrid();
	sColour = pcPainter->GetColours()->msText;

	PaintDashedLine(pcPainter, msStart.x, msStart.y, msEnd.x, msStart.y, SELECTION_DASH_PIXELS * fPixel, sColour);
	PaintDashedLine(pcPainter, msEnd.x, msStart.y, msEnd.x, msEnd.y, SELECTION_DASH_PIXELS * fPixel, sColour);
	PaintDashedLine(pcPainter, msEnd.x, msEnd.y, msStart.x, msEnd.y, SELECTION_DASH_PIXELS * fPixel, sColour);
	PaintDashedLine(pcPainter, msStart.x, msEnd.y, msStart.x, msStart.y, SELECTION_DASH_PIXELS * fPixel, sColour);

	x = msEnd.x + 2 * fPixel;
	y = msEnd.y;
	fBox = 11 * fPixel;
	if (msStart.x != msEnd.x)
	{
		pcPainter->Line(x, y - 15 * fPixel, x + fBox, y - 15 * fPixel, 1, sColour);
		pcPainter->Line(x + fBox, y - 15 * fPixel, x + fBox, y - 4 * fPixel, 1, sColour);
		pcPainter->Line(x + fBox, y - 4 * fPixel, x, y - 4 * fPixel, 1, sColour);
		pcPainter->Line(x, y - 4 * fPixel, x, y - 15 * fPixel, 1, sColour);
		if (msStart.x < msEnd.x)
		{
			pcPainter->Circle(x + 5.5 * fPixel, y - 9.5 * fPixel, 3.5 * fPixel, true, false, 1, sColour, sColour);
			x += 15 * fPixel;
		}
		else
		{
			pcPainter->Circle(x + 10.5 * fPixel, y - 9.5 * fPixel, 3.5 * fPixel, true, false, 1, sColour, sColour);
			x += 17 * fPixel;
		}
	}

	eMode = CalculateSelectionMode(mpcEditor->IsControl(), mpcEditor->IsShift());
	if ((eMode == SM_Add) || (eMode == SM_Subtract))
	{
		pcPainter->Line(x, y - 10 * fPixel, x + 10 * fPixel, y - 10 * fPixel, 1, sColour);
	}
	if (eMode == SM_Add)
	{
		pcPainter->Line(x + 5 * fPixel, y - 15 * fPixel, x + 5 * fPixel, y - 5 * fPixel, 1, sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSelectionEdit::CanTransformComponents(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStartEditInComponent::Init(CComponentView* pcPressed)
{
	mpcPressed = pcPressed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStartEditInComponent::Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Dragging moves the selection, which is just the pressed component if
// there is one.
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CStartEditInComponent::Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	CMoveComponentsEdit*	pcMove;
	CCircuitSelection*		pcSelection;

	pcSelection = pcEdit->GetEditor()->GetSelection();
	if (mpcPressed != NULL)
	{
		pcSelection->Clear();
		pcSelection->Add(mpcPressed);
	}
	pcMove = NewMalloc<CMoveComponentsEdit>();
	pcMove->Init(pcSelection->GetComponents(), pcSelection->GetTraces(), false, pcEdit->GetEditor()->IsControl());
	return pcMove;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStatefulEdit* CStartEditInComponent::Transform(CCircuitEdit* pcEdit)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
// A click on a selected view selects as a click anywhere does.  A changed
// selection can be undone.
//
//////////////////////////////////////////////////////////////////////////
void CStartEditInComponent::Done(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit)
{
	CCircuitSelection	cPrevious;

	cPrevious.Init();
	cPrevious.Set(pcEdit->GetEditor()->GetSelection());
	if (pcEdit->GetEditor()->SelectInRectangle(sPosition, sPosition, &cPrevious))
	{
		pcEdit->GetEditor()->PushUndo(true);
	}
	cPrevious.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStartEditInComponent::Discard(CCircuitEdit* pcEdit)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CStartEditInComponent::CanTransformComponents(void)
{
	return true;
}


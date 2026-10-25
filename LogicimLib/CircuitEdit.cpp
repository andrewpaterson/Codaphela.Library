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
#include "BaseLib/PointerFunctions.h"
#include "CircuitView.h"
#include "CircuitEdit.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStatefulEdit::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Most edits draw nothing of their own.
//
//////////////////////////////////////////////////////////////////////////
void CStatefulEdit::Paint(CCircuitPainter* pcPainter)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The edit owns the stateful edit (made with NewMalloc), which is
// started.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEdit::Init(SFloat64Vec2 sStart, CStatefulEdit* pcEdit, CCircuitEditor* pcEditor)
{
	mpcEdit = pcEdit;
	mpcEditor = pcEditor;
	msStart.Init(RoundToGrid(sStart.x), RoundToGrid(sStart.y));
	msDiff.Init(0, 0);
	mbHadDiff = false;
	miRightRotations = 0;

	mpcEdit->Start(sStart, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEdit::Kill(void)
{
	SafeKill(mpcEdit);
	mpcEditor = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing happens until the pointer has moved a grid point from where the
// edit started (or the edit has turned), so a click doesn't move.  A
// stateful edit handed on to is started and then moved too (Logicim's
// waits for the next move).
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEdit::Move(SFloat64Vec2 sPosition)
{
	CStatefulEdit*	pcNewEdit;
	SFloat64Vec2	sStart;

	msDiff.Init(RoundToGrid(sPosition.x) - msStart.x, RoundToGrid(sPosition.y) - msStart.y);
	if (HasDiff())
	{
		mbHadDiff = true;
	}

	if (mbHadDiff)
	{
		pcNewEdit = mpcEdit->Move(sPosition, this);
		if (pcNewEdit != mpcEdit)
		{
			SafeKill(mpcEdit);
			mpcEdit = pcNewEdit;
			sStart.Init(msStart.x, msStart.y);
			mpcEdit->Start(sStart, this);
			mpcEdit->Move(sPosition, this);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Turns a quarter right (clockwise) or left about the start.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEdit::Rotate(bool bRight)
{
	CStatefulEdit*	pcNewEdit;

	mbHadDiff = true;
	miRightRotations = (miRightRotations + (bRight ? 1 : 3)) % 4;

	pcNewEdit = mpcEdit->Rotate(bRight, this);
	if (pcNewEdit != mpcEdit)
	{
		SafeKill(mpcEdit);
		mpcEdit = pcNewEdit;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEdit::Done(SFloat64Vec2 sPosition)
{
	mpcEdit->Done(sPosition, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEdit::Discard(void)
{
	mpcEdit->Discard(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEdit::Paint(CCircuitPainter* pcPainter)
{
	mpcEdit->Paint(pcPainter);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the pointer has moved from the start or the edit has turned.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEdit::HasDiff(void)
{
	return (msDiff.x != 0) || (msDiff.y != 0) || (miRightRotations != 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCircuitEdit::GetRightRotations(void) { return miRightRotations; }
SInt32Vec2 CCircuitEdit::GetStart(void) { return msStart; }
SInt32Vec2 CCircuitEdit::GetDiff(void) { return msDiff; }
CCircuitEditor* CCircuitEdit::GetEditor(void) { return mpcEditor; }
bool CCircuitEdit::CanTransformComponents(void) { return mpcEdit->CanTransformComponents(); }


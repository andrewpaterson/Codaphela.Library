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
#include "Canvas.h"
#include "TextMarksCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextMarksCanvasDraw::Init(ARGB32 sSelectionColour, ARGB32 sCaretColour)
{
	PreInit();
	CCanvasDraw::Init();
	msSelectionColour = sSelectionColour;
	msCaretColour = sCaretColour;
	mcSelection.Init(0, 0, 0, 0);
	mcCaret.Init(0, 0, 0, 0);
	mbSelection = false;
	mbCaret = false;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextMarksCanvasDraw::Class(void)
{
	CCanvasDraw::Class();
	U_Data(CInt32Rect, mcSelection);
	U_Data(CInt32Rect, mcCaret);
	U_UInt32(msSelectionColour);
	U_UInt32(msCaretColour);
	U_Bool(mbSelection);
	U_Bool(mbCaret);
}


//////////////////////////////////////////////////////////////////////////
//
// The canvas is transparent so only the marks cover what is behind it.
//
//////////////////////////////////////////////////////////////////////////
bool CTextMarksCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
	if (pCanvas->IsValid())
	{
		if (mbSelection)
		{
			pCanvas->DrawBox(&mcSelection, true, msSelectionColour);
		}
		if (mbCaret)
		{
			pCanvas->DrawBox(&mcCaret, true, msCaretColour);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The rectangle includes its right and bottom edges.
//
//////////////////////////////////////////////////////////////////////////
void CTextMarksCanvasDraw::SetSelection(int32 iLeft, int32 iTop, int32 iRight, int32 iBottom)
{
	mcSelection.Init(iLeft, iTop, iRight, iBottom);
	mbSelection = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The rectangle includes its right and bottom edges.
//
//////////////////////////////////////////////////////////////////////////
void CTextMarksCanvasDraw::SetCaret(int32 iLeft, int32 iTop, int32 iRight, int32 iBottom)
{
	mcCaret.Init(iLeft, iTop, iRight, iBottom);
	mbCaret = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextMarksCanvasDraw::HideSelection(void) { mbSelection = false; }
void CTextMarksCanvasDraw::HideCaret(void) { mbCaret = false; }
bool CTextMarksCanvasDraw::IsSelectionShown(void) { return mbSelection; }
bool CTextMarksCanvasDraw::IsCaretShown(void) { return mbCaret; }
void CTextMarksCanvasDraw::GetSelection(CInt32Rect* pcDest) { *pcDest = mcSelection; }
void CTextMarksCanvasDraw::GetCaret(CInt32Rect* pcDest) { *pcDest = mcCaret; }


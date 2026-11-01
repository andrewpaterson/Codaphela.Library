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
#ifndef __TEXT_MARKS_CANVAS_DRAW_H__
#define __TEXT_MARKS_CANVAS_DRAW_H__
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Int32Rect.h"
#include "CanvasDraw.h"


//Draws the marks an edited line of text has under it: the selection's block and the caret.  The text field that owns
//it places them, in the canvas' space, each time it is laid out.  Either can be hidden.
class CTextMarksCanvasDraw : public CCanvasDraw
{
CONSTRUCTABLE(CTextMarksCanvasDraw);
DESTRUCTABLE(CTextMarksCanvasDraw);
protected:
	CInt32Rect	mcSelection;  //Includes its right and bottom edges, as DrawBox does.
	CInt32Rect	mcCaret;
	ARGB32		msSelectionColour;
	ARGB32		msCaretColour;
	bool		mbSelection;
	bool		mbCaret;

public:
	void	Init(ARGB32 sSelectionColour, ARGB32 sCaretColour);
	void	Class(void) override;

	bool	Draw(Ptr<CCanvas> pCanvas) override;

	void	SetSelection(int32 iLeft, int32 iTop, int32 iRight, int32 iBottom);
	void	HideSelection(void);
	void	SetCaret(int32 iLeft, int32 iTop, int32 iRight, int32 iBottom);
	void	HideCaret(void);
	bool	IsSelectionShown(void);
	bool	IsCaretShown(void);
	void	GetSelection(CInt32Rect* pcDest);
	void	GetCaret(CInt32Rect* pcDest);
};


#endif // __TEXT_MARKS_CANVAS_DRAW_H__


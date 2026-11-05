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
#ifndef __CODE_EDITOR_H__
#define __CODE_EDITOR_H__
#include "BaseLib/ArrayTemplate.h"
#include "SupportLib/FontFamily.h"
#include "Container.h"
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "TextArea.h"
#include "ComponentEvent.h"


typedef CArrayTemplate<uint32>	CArrayCodeEditorGapIds;
typedef CArrayTemplate<size>	CArrayCodeEditorLines;
typedef CArrayTemplate<char>	CArrayCodeEditorMarks;


//Lines that changed, like those changed since a file was committed, shown by a bar in the gutter, or by the whole of
//their rows of the gutter (a band, as a diff view shows them).  Lines uiStartLine up to uiEndLine changed; with
//uiStartLine equal to uiEndLine, lines were removed above line uiStartLine.
struct SCodeEditorChangeBar
{
	size	uiStartLine;
	size	uiEndLine;
	ARGB32	sColour;
	bool	bBand;
};


typedef CArrayTemplate<SCodeEditorChangeBar>	CArrayCodeEditorChangeBars;


#define CODE_EDITOR_REMOVED_THICKNESS	2


//A press in the gutter, and the line it was next to.
class CCodeEditor;
typedef CComponentValueEvent<CCodeEditor, size>	CGutterClickEvent;


//Source code to edit: a gutter of line numbers on the left of a text area (GetTextArea) of the code, and components
//shown under lines of the code (AddLineComponent), each in a gap the text area leaves below its line, as wide as the
//code and as high as the component wants.  A line component stays with its line as the code is edited and goes when
//its line is deleted.  The gutter is a text area too, in the code's font with the same gaps, so its numbers are on
//the code's rows; the code isn't word wrapped and its spans shouldn't change the font's size or the rows wouldn't
//match.  The gutter's first column has a mark for each line (SetLineMark), a space unless set; marks are by line
//number, so they don't move with the lines as the code is edited.  Change bars (AddChangeBar) colour the gutter's last
//column beside lines that changed, or draw a line between rows where lines were removed; they are by line number too.
//Change bands (AddChangeBand) colour the gutter's whole rows instead.
//Clicking the gutter calls the gutter click event
//with the line it was next to, and an edit calls the change event.  The code editor is meant to be the content of a
//CScrollPane: it is as big as the gutter and code need, and the text area keeps its caret in view.  Scrolled sideways
//the gutter stays at the view's left, on its own background over the code, which goes under it; the caret is kept
//in view right of it.
class CCodeEditor : public CContainer
{
CONSTRUCTABLE(CCodeEditor);
DESTRUCTABLE(CCodeEditor);
protected:
	Ptr<CTextArea>			mpGutter;
	Ptr<CCanvas>			mpGutterBackground;
	Ptr<CClearCanvasDraw>	mpGutterDraw;
	Ptr<CTextArea>			mpTextArea;
	CArray<CComponent>		maLineComponents;
	CArrayCodeEditorGapIds	mauiGapIds;  //Of each line component's gap in the text area.  Ascending: a new gap's id is the largest.
	CArrayCodeEditorLines	mauiLines;  //Each line component's line, as of the last update.
	size					muiGutterLines;  //The lines the gutter was last numbered for.
	CArrayCodeEditorMarks	macMarks;  //By line; lines past its end, and '\0's, have none.
	bool					mbMarksChanged;  //Since the gutter was last numbered.
	CArrayCodeEditorChangeBars	masChangeBars;
	bool					mbChangeBarsChanged;  //Since they were last drawn on the gutter.
	size					muiSeenChanges;  //The text area's change count when the change event was last called.
	CGutterClickEvent		mcGutterClicked;
	CComponentEvent<CCodeEditor>	mcChanged;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CFontFamily> pFamily, const char* szText);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CTextArea>		GetTextArea(void);
	Ptr<CTextArea>		GetGutter(void);
	void				SetGutterColour(ARGB32 sColour);
	void				SetGutterBackground(ARGB32 sColour);
	int32				GetGutterWidth(void);

	bool				AddLineComponent(size uiLine, Ptr<CComponent> pComponent);
	bool				RemoveLineComponent(Ptr<CComponent> pComponent);
	void				RemoveAllLineComponents(void);
	size				NumLineComponents(void);
	Ptr<CComponent>		GetLineComponent(size uiIndex);
	size				GetLineOf(Ptr<CComponent> pComponent);
	Ptr<CComponent>		GetComponentOn(size uiLine);

	void				SetLineMark(size uiLine, char cMark);
	char				GetLineMark(size uiLine);
	void				ClearLineMarks(void);
	void				AddChangeBar(size uiStartLine, size uiEndLine, ARGB32 sColour);
	void				AddChangeBand(size uiStartLine, size uiEndLine, ARGB32 sColour);
	void				ClearChangeBars(void);
	size				NumChangeBars(void);
	SCodeEditorChangeBar*	GetChangeBar(size uiIndex);
	CGutterClickEvent*	GetGutterClickEvent(void);
	CComponentEvent<CCodeEditor>*	GetChangeEvent(void);
	void				ClickAction(SComponentInputEvent* psEvent);

	void				SetRequiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	void				ScrollToShow(CInt32Rect* pcRect) override;

protected:
	void				BringGutterToFront(void);
	size				FindLineComponent(Ptr<CComponent> pComponent);
	size				FindGapId(uint32 uiId);
	void				RemoveLineComponentAt(size uiIndex);
	int32				GetGapHeight(Ptr<CComponent> pComponent);
	void				Update(void);
	void				UpdateLineComponents(void);
	void				UpdateGutter(void);
	void				DrawChangeBars(size uiDigits);
	void				AddChangeBar(size uiStartLine, size uiEndLine, ARGB32 sColour, bool bBand);
	size				CountDigits(size uiNumber);
};


#endif // __CODE_EDITOR_H__


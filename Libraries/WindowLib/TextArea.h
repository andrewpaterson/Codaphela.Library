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
#ifndef __TEXT_AREA_H__
#define __TEXT_AREA_H__
#include "BaseLib/TextEditing.h"
#include "SupportLib/Font.h"
#include "SupportLib/FontColours.h"
#include "SupportLib/FontFamily.h"
#include "SupportLib/TextStyle.h"
#include "SupportLib/GeometryMap.h"
#include "SupportLib/Maps.h"
#include "Canvas.h"
#include "TextBackgrounds.h"
#include "TextAreaLayout.h"
#include "TextAreaBands.h"
#include "TextAreaShapes.h"
#include "TextEditingInput.h"
#include "ComponentInputEvent.h"
#include "ComponentEvent.h"
#include "WindowTheme.h"


class CTextArea;
typedef CComponentValueEvent<CTextArea, STextUndoChange*>	CTextAreaUndoEvent;  //What happened to the undo history.


//Lines of text drawn with a CMapsCanvasDraw.  Line feeds start new lines and, with word wrap on (it is off by default),
//lines are also broken at spaces to fit the width the text area is laid out to.  A word too wide for a line by itself
//is broken between glyphs.  Like a label the text area is a transparent canvas with its text at the top left; text that
//doesn't fit is clipped.  A word wrapped text area is variable sized horizontally: it takes the width its container
//gives it.  Without word wrap its lines can't be made narrower so its required width is the width of the laid out text.
//Its required height is the height of the laid out lines, wrapped or not.
//
//The text is drawn in the faces, sizes and colours of a font family (see CFontFamily), and can be underlined, overlined
//and struck through in its colour.  Text is in the text area's style (regular, the family's default size, opaque white
//unless set) except where spans override parts of it; where spans overlap each property takes the value of the last
//span added that sets it.  Spans are applied when the text area is next laid out, drawn or asked its size.
//
//The text is kept in a CTextDocument and edited by a CTextEditing, like IntelliJ's editor but for text in proportional
//fonts and styles: positions are bytes of the UTF8 text, always at the start of a character (a ZWJ sequence is one
//character), and the spans move and stretch with the text so new text takes the style of the character before it.  The
//text area is the editing's view: up, down, home and end work on the rows it lays the text out in, so on wrapped lines
//too.  It is meant to be the content of a CScrollPane: the caret is kept in view and a page is the view's height.  Like
//CTextField's, the caret is only drawn while the text area has focus; the selection is drawn under the text.  A text
//area that isn't editable can still be navigated, selected and copied from.
//
//Lines (ended by line feeds, numbered from 0) can have an empty gap below them (SetLineGap) for whatever owns the
//text area to draw something in, such as the code a line compiled to.  A gap goes under the last row of a wrapped
//line; in tiles it is rounded up to whole rows.  The caret and selection go over gaps as if they weren't there.  A gap
//stays with its line as text is edited: it moves down with a line pushed down by line feeds put in at its start, and
//goes with a line that is deleted or joined onto the line above.  Each gap has an id (GetLineGapId) for finding the
//line it is with after edits (FindLineGap); it keeps it while it is there, through changes of height too.
//
//Colours can be drawn under the text, under the selection too: over ranges of the text (AddBackground), whole lines
//across the text area's width (AddLineBackground) and thin lines between lines (AddLineBoundary), each drawn over the
//ones added before it.  Line backgrounds don't cover the gaps below lines.  Ranges move with the text and grow with
//text put strictly inside them; line backgrounds and boundaries stay with their lines as gaps do.  Setting the text
//removes them.
class CTextArea : public CCanvas, public CTextEditingView
{
CONSTRUCTABLE(CTextArea);
DESTRUCTABLE(CTextArea);
protected:
	CTextDocument			mcDocument;  //The text, its lines and its undo history.
	Ptr<CTextAreaLayout>	mpLayout;  //The text in rows, as sprites or tiles, in its styles and with the gaps below its lines.
	Ptr<CMaps>				mpMaps;  //The backgrounds, selection, text and the lines along it, in that order.
	Ptr<CGeometryMap>		mpBackgroundMap;  //Under the selection.
	CTextBackgrounds		mcBackgrounds;  //The colours under the text (AddBackground).
	bool					mbBackgroundsDirty;  //The backgrounds or the layout changed since the map was drawn.
	int32					miBackgroundWidth;  //The width the map was drawn across.
	Ptr<CGeometryMap>		mpSelectionMap;  //Under the text.
	CTextAreaShapes			mcShapes;  //Draws the backgrounds and the selection into their maps.
	bool					mbRowsChanged;  //Something drawn on the rows may have changed since the text area was drawn.
	CTextAreaBands			mcBands;  //What was drawn, row by row, to find what changed (PrepareArea).
	ARGB32					msSelectionColour;
	ARGB32					msCaretColour;
	bool					mbCaretLine;  //The caret's line is drawn under the other backgrounds, in msCaretLineColour.
	ARGB32					msCaretLineColour;
	size					muiCaretLine;  //The line it was drawn on.
	CTextEditing			mcEditing;  //The caret, the selection and the edits of mcDocument.
	bool					mbSelectable;  //The pointer puts the caret and selects; otherwise its presses pass to the parent.
	CTextEditingInput		mcInput;  //The input actions that reach mcEditing.

	//Undo, as Logicim's: the text after each edit of an editable text area, consecutive typing kept as one edit (see
	//CTextUndo).  Undoing or redoing changes only the part of the text that differs from the snapshot (see
	//CTextDocument::Load).
	CTextAreaUndoEvent			mcUndoEvent;
	CComponentEvent<CTextArea>	mcChanged;  //Called after each edit, as the change count goes up.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pWhiteFont, const char* szText);
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFontColours> pColours, const char* szText);
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFontFamily> pFamily, const char* szText);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void						SetText(const char* szText);
	void						AppendText(const char* szText, ARGB32 sColour);
	char*						GetText(void);
	size						GetChangeCount(void);
	bool						Undo(void);
	bool						Redo(void);
	bool						CanUndo(void);
	bool						CanRedo(void);
	size						GetUndoId(void);
	void						ClearUndo(void);
	bool						UndoTo(size uiId);
	void						SetTypingPause(int32 iMilliseconds);
	CTextAreaUndoEvent*			GetUndoEvent(void);
	CComponentEvent<CTextArea>*	GetChangeEvent(void);
	void						SetFont(Ptr<CFont> pWhiteFont);
	Ptr<CFont>					GetFont(void);
	int32						GetTabPixels(void);
	void						SetFontColours(Ptr<CFontColours> pColours);
	Ptr<CFontColours>			GetFontColours(void);
	void						SetFontFamily(Ptr<CFontFamily> pFamily);
	Ptr<CFontFamily>			GetFontFamily(void);
	void						SetWordWrap(bool bWordWrap);
	bool						IsWordWrap(void);
	void						SetTiledText(bool bTiledText);
	bool						IsTiledText(void);
	bool						IsDrawnAsTiles(void);

	void					SetStyle(STextStyle* psStyle);
	void					GetStyle(STextStyle* psStyle);
	void					SetColour(ARGB32 sColour);
	ARGB32					GetColour(void);
	bool					AddSpan(size uiStart, size uiLength, ARGB32 sColour);
	bool					AddSpan(size uiStart, size uiLength, STextStyleOverride* psOverride);
	void					ClearSpans(void);
	size					NumSpans(void);
	bool					AddBackground(size uiStart, size uiEnd, ARGB32 sColour);
	bool					AddLineBackground(size uiStartLine, size uiEndLine, ARGB32 sColour);
	void					AddDocumentListener(CTextDocumentListener* pcListener);
	void					RemoveDocumentListener(CTextDocumentListener* pcListener);
	bool					AddLineBoundary(size uiLine, int32 iThickness, ARGB32 sColour);
	void					ClearBackgrounds(void);
	size					NumBackgrounds(void);
	STextAreaBackground*	GetBackground(size uiIndex);
	void					GetStyleAt(size uiPosition, STextStyle* psStyle);
	ARGB32					GetColourAt(size uiPosition);

	void				SetEditable(bool bEditable);
	bool				IsEditable(void);
	void				SetSelectable(bool bSelectable);
	bool				IsSelectable(void);
	void				SetTabWidth(int32 iTabWidth);
	void				SetMarkColours(ARGB32 sSelection, ARGB32 sCaret);
	void				SetCaretLine(bool bShown, ARGB32 sColour);
	bool				IsCaretLineShown(void);
	size				GetCaretLine(void);
	size				GetCaretPosition(void);
	void				SetCaretPosition(size uiPosition);
	bool				GetSelection(size* puiStart, size* puiEnd);
	void				Select(size uiStart, size uiEnd);
	size				FindPositionAt(int x, int y);
	bool				FindCharacterAt(int x, int y, size* puiPosition);
	bool				StartLineDrag(int x, int y, bool bExtend);
	void				GetCaretBounds(CInt32Rect* pcDest);
	bool				GetCaretRect(CInt32Rect* pcDest, ARGB32* psColour) override;

	//Lines and the gaps below them.
	size					NumLines(void);
	size					GetLineStart(size uiLine);
	size					GetLineEnd(size uiLine);
	size					FindLine(size uiPosition);
	bool					GetLineBounds(size uiLine, CInt32Rect* pcDest);
	bool					SetLineGap(size uiLine, int32 iHeight);
	int32					GetLineGap(size uiLine);
	size					NumLineGaps(void);
	void					ClearLineGaps(void);
	bool					GetLineGapBounds(size uiLine, CInt32Rect* pcDest);
	uint32					GetLineGapId(size uiLine);
	size					FindLineGap(uint32 uiId);
	CArrayTextLayoutGap*	GetLineGaps(void);
	void					SetLineGaps(CArrayTextLayoutGap* pasGaps);

	//The edits and moves of the text area's CTextEditing.
	void				Up(void);
	void				Down(void);
	void				Left(void);
	void				Right(void);
	void				WordLeft(void);
	void				WordRight(void);
	void				Home(void);
	void				End(void);
	void				DocumentHome(void);
	void				DocumentEnd(void);
	void				PageUp(void);
	void				PageDown(void);
	void				SelectUp(void);
	void				SelectDown(void);
	void				SelectLeft(void);
	void				SelectRight(void);
	void				SelectWordLeft(void);
	void				SelectWordRight(void);
	void				SelectHome(void);
	void				SelectEnd(void);
	void				SelectDocumentHome(void);
	void				SelectDocumentEnd(void);
	void				SelectPageUp(void);
	void				SelectPageDown(void);
	void				SelectWholeWord(void);
	void				SelectAll(void);
	void				ClearSelection(void);
	void				Printable(char c, bool bInsert);
	void				Tab(bool bInsert);
	void				BackTab(void);
	void				Enter(void);
	void				HomeEnter(void);
	void				EndEnter(void);
	void				Delete(void);
	void				Backspace(void);
	void				Cut(CChars* pszDest);
	void				Copy(CChars* pszDest);
	void				Paste(char* szSource);
	bool				ReplaceRanges(size* auiStarts, size* auiEnds, size uiNumRanges, const char* szText);
	void				CutLine(CChars* pszDest);
	void				Duplicate(void);
	void				DeleteWordRemainingRight(void);
	void				DeleteWordRemainingLeft(void);
	void				DeleteLine(void);

	void				LineUp(void);
	void				LineDown(void);

	//Cut, copy and paste with the window's clipboard.
	void				CutToClipboard(void);
	void				CopyToClipboard(void);
	void				PasteFromClipboard(void);

	SInt32Vec2			GetTextSize(void);
	SInt32Vec2			GetRequiredSize(void);
	SInt32Vec2			GetDesiredSize(void) override;

	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool				Draw(void) override;
	bool				WantsEnter(void) override;
	bool				UsesEditorKeys(void) override;
	bool				CanDoCommand(EComponentInputCommand eCommand) override;
	SInt32Vec2			GetContextMenuPosition(void) override;

	void				FocusGained(void) override;
	void				FocusLost(void) override;

	//The editing's view (CTextEditingView): the rows the text is laid out in.
	bool				HasRows(void) override;
	size				NumRows(void) override;
	size				FindRow(size uiPosition, bool bRowEnd) override;
	size				GetRowStart(size uiRow) override;
	size				GetRowEnd(size uiRow, bool* pbRowEnd) override;
	size				FindRowAt(int32 y) override;
	size				FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd) override;
	int32				GetPositionX(size uiPosition, bool bRowEnd) override;
	size				ScrollPage(size uiRow, bool bDown) override;
	void				MarksChanged(void) override;
	void				ShowCaret(void) override;
	void				UpdateCaretLine(void);
	void				TextEdited(void) override;
	void				UndoChanged(STextUndoChange* psChange) override;

protected:
	void				PrepareArea(bool bDrawn, bool bClipped, CInt32Rect* pcClip, CInt32Rect* pcArea) override;
	void				Relayout(void);
	void				UpdateText(void);
	void				UpdateSize(void);
	void				LaidOut(void);
	void				GapsChanged(void);

	//The colours under the text.
	void				UpdateBackgrounds(void);
	int32				GetFullWidth(void);

	int32				GetPageHeight(void);
	void				ScrollBy(int32 iDown);

	void				UpdateMarks(void);
	void				RowsChanged(void);
	void				BackgroundsChanged(void);
	bool				SummariseBands(CTextAreaBands* pcBands);
};


#endif // __TEXT_AREA_H__


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
#include "BaseLib/UndoStack.h"
#include "SupportLib/Font.h"
#include "SupportLib/FontColours.h"
#include "SupportLib/FontFamily.h"
#include "SupportLib/TextStyle.h"
#include "SupportLib/Text.h"
#include "SupportLib/TextLayout.h"
#include "SupportLib/SimpleTextReader.h"
#include "SupportLib/SpriteMap.h"
#include "SupportLib/GeometryMap.h"
#include "SupportLib/MapFontDraw.h"
#include "SupportLib/Maps.h"
#include "SupportLib/TileMap.h"
#include "SupportLib/TileLayerCel.h"
#include "SupportLib/TileTextWriter.h"
#include "Canvas.h"
#include "ComponentInputEvent.h"
#include "ComponentEvent.h"


//How long typing can stop for and the next character still be part of the same edit (see CTextArea::KeepEdit).
#define TEXT_AREA_TYPING_PAUSE	1000


//What happened to a text area's undo history (CTextArea::GetUndoEvent).
enum ETextAreaUndo
{
	TAU_Kept,	//An edit was kept: the text went from state uiIdBefore to the new state uiIdAfter.
	TAU_Merged,	//An edit was kept as part of the one before (typing a word): state uiIdBefore became uiIdAfter.
	TAU_Undone,	//The text went back from state uiIdBefore to uiIdAfter.
	TAU_Redone,	//The text went forward from state uiIdBefore to uiIdAfter.
};


struct STextAreaUndo
{
	ETextAreaUndo	eChange;
	size			uiIdBefore;
	size			uiIdAfter;
};


class CTextArea;
typedef CComponentValueEvent<CTextArea, STextAreaUndo*>	CTextAreaUndoEvent;


//A range of the text in a style.  uiStart and uiLength are in bytes of the UTF8 text.
struct STextAreaSpan
{
	size				uiStart;
	size				uiLength;
	STextStyleOverride	sOverride;
};


typedef CArrayTemplate<STextAreaSpan>	CArrayTextAreaSpan;


enum ETextAreaBackground
{
	TAB_Range,  //Bytes [uiStart, uiEnd) of the UTF8 text, on each row as the selection is.
	TAB_Lines,  //Lines [uiStart, uiEnd), across the text area's whole width.
	TAB_Boundary  //A line iThickness high across the whole width, centred on the top of line uiStart (or the bottom of the last line, for NumLines), kept inside the text.
};


//A colour drawn under the text (see AddBackground, AddLineBackground and AddLineBoundary).
struct STextAreaBackground
{
	ETextAreaBackground	eType;
	size				uiStart;
	size				uiEnd;
	int32				iThickness;
	ARGB32				sColour;
};


typedef CArrayTemplate<STextAreaBackground>	CArrayTextAreaBackground;
typedef CArrayTemplate<size>			CArrayTextAreaStopStart;
typedef CArrayTemplate<size>			CArrayTextAreaLineStart;


//Where up and down move the caret: a row or a page of rows.
enum ETextAreaMove
{
	TAM_RowUp,
	TAM_RowDown,
	TAM_PageUp,
	TAM_PageDown,
};


//Lines of text drawn with a CMapsCanvasDraw.  Line feeds start
//new lines and, with word wrap on (it is off by default), lines are also broken at spaces to fit the width the text
//area is laid out to.  A word too wide for a line by itself is broken between glyphs.  Like a label the text area is
//a transparent canvas with its text at the top left; text that doesn't fit is clipped.  A word wrapped text area is
//variable sized horizontally: it takes the width its container gives it.  Without word wrap its lines can't be made
//narrower so its required width is the width of the laid out text.  Its required height is the height of the laid
//out lines, wrapped or not.
//
//The text is drawn in the faces, sizes and colours of a font family (see CFontFamily), and can be underlined, overlined
//and struck through in its colour.  Text is in the text area's
//style (regular, the family's default size, opaque white unless set) except where spans override parts of it; where
//spans overlap each property takes the value of the last span added that sets it.  Spans are applied when the text
//area is next laid out, drawn or asked its size.
//
//The text is edited with the functions of a CTextEditor, like IntelliJ's editor but for text in proportional fonts and
//styles: positions are bytes of the UTF8 text, always at the start of a character (a ZWJ sequence is one character),
//and the spans move and stretch with the text so new text takes the style of the character before it.  Up, down,
//home and end work on the rows the text is laid out in, so on wrapped lines too; up and down keep the caret's x and
//home is smart, it goes to the first character that isn't a space and then to the start of the line, and a second
//home or end on a wrapped row goes to the ends of the line.  Tab inserts spaces to the next tab stop, or with a
//selection over more than one line indents its lines; back tab removes a tab's worth of spaces from them.  The text
//area is meant to be the content of a CScrollPane: the caret is kept in view and a page is the view's height.  Like
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
class CTextArea : public CCanvas
{
CONSTRUCTABLE(CTextArea);
DESTRUCTABLE(CTextArea);
protected:
	CChars				mszText;
	Ptr<CFontFamily>	mpFamily;
	STextStyle			msStyle;
	CArrayTextAreaSpan	masSpans;
	bool				mbTextDirty;  //The spans or style changed since the text was last read.
	bool				mbSizeDirty;  //Reading the text again can change its size.
	size				muiChangeCount;  //How many times the text has been edited (GetChangeCount).
	Ptr<CText>			mpText;
	bool				mbTextStale;  //Text was appended after mpText was read (AppendSprites): read it again before it is laid out whole.
	Ptr<CTextLayout>	mpLayout;
	Ptr<CTextLayout>	mpAppendLayout;  //Lays out text appended, from the start of its first line (AppendSprites).  NULL until then.
	Ptr<CSpriteMap>		mpSpriteMap;
	Ptr<CGeometryMap>	mpGeometryMap;  //The lines drawn along the text, over it.
	Ptr<CMapFontDraw>	mpFontDraw;
	Ptr<CMaps>			mpMaps;
	bool				mbWordWrap;
	SInt32Vec2			msTextSize;

	//A fixed width font can be drawn as tiles instead of sprites: each character in a cell of a tile layer, the lines
	//and stops worked out from the columns instead of by mpLayout.
	bool					mbTiledText;  //Tiles were asked for; they are only used with a fixed width font and no word wrap.
	bool					mbTiled;  //The text was last read as tiles.
	Ptr<CTileMap>			mpTileMap;
	Ptr<CTileLayerCel>		mpTileLayer;
	Ptr<CTileTextWriter>	mpTileWriter;
	CArrayTextLayoutLine	masTileLines;
	CArrayTextLayoutStop	masTileStops;

	Ptr<CGeometryMap>	mpBackgroundMap;  //Under the selection.
	CArrayTextAreaBackground	masBackgrounds;
	bool				mbBackgroundsDirty;  //The backgrounds or the layout changed since the map was drawn.
	int32				miBackgroundWidth;  //The width the map was drawn across.
	Ptr<CGeometryMap>	mpSelectionMap;  //Under the text.
	ARGB32				msSelectionColour;
	ARGB32				msCaretColour;
	CArrayTextAreaStopStart	mauiStopStarts;  //The byte each of the layout's stops is before.  Carriage returns have none.
	CArrayTextAreaLineStart	mauiLineStarts;  //The first byte of each line, kept as the text is edited.
	CArrayTextLayoutGap	masGaps;  //Sorted by line, at most one a line.
	uint32				muiNextGapId;  //The id the next new gap gets, from 1.
	size				muiCaret;
	size				muiAnchor;  //Where the selection started, MAX_SIZE for no selection.
	bool				mbCaretAtRowEnd;  //A caret at the start of a wrapped row is drawn at the end of the row above.
	int32				miUpDownX;  //Where up and down try to keep the caret, -1 until they are used.
	int32				miTabWidth;  //In spaces.
	bool				mbEditable;
	bool				mbSelectable;  //The pointer puts the caret and selects; otherwise its presses pass to the parent.
	bool				mbDragging;

	//Undo, as Logicim's: the text after each edit of an editable text area (CUndoStack), consecutive typing kept as one
	//edit.  Undoing or redoing changes only the part of the text that differs from the snapshot.
	CUndoStack			mcUndo;
	bool				mbUndoing;  //An undo or redo is changing the text: it isn't an edit to keep.
	bool				mbTyping;  //The edit being made is a character typed (Printable).
	bool				mbLastTyping;  //So was the last edit kept.
	char				mcTyped;  //The character typed.
	char				mcLastTyped;
	size				muiTypedAt;  //Where the caret was when it was typed.
	size				muiTypedEnd;  //Where the caret was after the last edit kept.
	int64				miTypedTime;  //When the last character was typed (GetDebugTickCount).
	int32				miTypingPause;  //Typing that stops this long (milliseconds) starts a new edit.
	bool				mbEditStarted;  //The edit being made has changed the text: muiEditCaret and muiEditAnchor are from before it.
	size				muiEditCaret;
	size				muiEditAnchor;
	CTextAreaUndoEvent	mcUndoEvent;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CFont> pWhiteFont, const char* szText);
	void				Init(Ptr<CWindow> pWindow, Ptr<CFontColours> pColours, const char* szText);
	void				Init(Ptr<CWindow> pWindow, Ptr<CFontFamily> pFamily, const char* szText);
	void				Class(void) override;
	void 				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetText(const char* szText);
	void				AppendText(const char* szText, ARGB32 sColour);
	char*				GetText(void);
	size				GetChangeCount(void);
	bool				Undo(void);
	bool				Redo(void);
	bool				CanUndo(void);
	bool				CanRedo(void);
	size				GetUndoId(void);
	void				ClearUndo(void);
	bool				UndoTo(size uiId);
	void				SetTypingPause(int32 iMilliseconds);
	CTextAreaUndoEvent*	GetUndoEvent(void);
	void				SetFont(Ptr<CFont> pWhiteFont);
	Ptr<CFont>			GetFont(void);
	int32				GetTabPixels(void);
	void				SetFontColours(Ptr<CFontColours> pColours);
	Ptr<CFontColours>	GetFontColours(void);
	void				SetFontFamily(Ptr<CFontFamily> pFamily);
	Ptr<CFontFamily>	GetFontFamily(void);
	void				SetWordWrap(bool bWordWrap);
	bool				IsWordWrap(void);
	void				SetTiledText(bool bTiledText);
	bool				IsTiledText(void);
	bool				IsDrawnAsTiles(void);

	void				SetStyle(STextStyle* psStyle);
	void				GetStyle(STextStyle* psStyle);
	void				SetColour(ARGB32 sColour);
	ARGB32				GetColour(void);
	bool				AddSpan(size uiStart, size uiLength, ARGB32 sColour);
	bool				AddSpan(size uiStart, size uiLength, STextStyleOverride* psOverride);
	void				ClearSpans(void);
	size				NumSpans(void);
	bool				AddBackground(size uiStart, size uiEnd, ARGB32 sColour);
	bool				AddLineBackground(size uiStartLine, size uiEndLine, ARGB32 sColour);
	bool				AddLineBoundary(size uiLine, int32 iThickness, ARGB32 sColour);
	void				ClearBackgrounds(void);
	size				NumBackgrounds(void);
	STextAreaBackground*	GetBackground(size uiIndex);
	void				GetStyleAt(size uiPosition, STextStyle* psStyle);
	ARGB32				GetColourAt(size uiPosition);

	void				SetEditable(bool bEditable);
	bool				IsEditable(void);
	void				SetSelectable(bool bSelectable);
	bool				IsSelectable(void);
	void				SetTabWidth(int32 iTabWidth);
	void				SetMarkColours(ARGB32 sSelection, ARGB32 sCaret);
	size				GetCaretPosition(void);
	void				SetCaretPosition(size uiPosition);
	bool				GetSelection(size* puiStart, size* puiEnd);
	void				Select(size uiStart, size uiEnd);
	size				FindPositionAt(int x, int y);
	void				GetCaretBounds(CInt32Rect* pcDest);
	bool				GetCaretRect(CInt32Rect* pcDest, ARGB32* psColour) override;

	//Lines and the gaps below them.
	size				NumLines(void);
	size				GetLineStart(size uiLine);
	size				FindLine(size uiPosition);
	bool				GetLineBounds(size uiLine, CInt32Rect* pcDest);
	bool				SetLineGap(size uiLine, int32 iHeight);
	int32				GetLineGap(size uiLine);
	size				NumLineGaps(void);
	void				ClearLineGaps(void);
	bool				GetLineGapBounds(size uiLine, CInt32Rect* pcDest);
	uint32				GetLineGapId(size uiLine);
	size				FindLineGap(uint32 uiId);
	CArrayTextLayoutGap*	GetLineGaps(void);
	void				SetLineGaps(CArrayTextLayoutGap* pasGaps);

	//The functions of a CTextEditor.
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
	void				UndoAction(void);
	void				RedoAction(void);
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
	void				CutLineToClipboard(void);
	void				PasteFromClipboard(void);

	SInt32Vec2			GetTextSize(void);
	SInt32Vec2			GetRequiredSize(void);
	SInt32Vec2			GetDesiredSize(void) override;

	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool				Draw(void) override;
	bool				WantsEnter(void) override;
	SInt32Vec2			GetContextMenuPosition(void) override;

	void				FocusGained(void) override;
	void				FocusLost(void) override;
	void				PrintableAction(SComponentInputEvent* psEvent);
	void				SpaceAction(SComponentInputEvent* psEvent);
	void				TabAction(SComponentInputEvent* psEvent);
	void				ActivateComponent(int x, int y);
	void				ContextMenuClick(SComponentInputEvent* psEvent);
	void				ExecuteComponent(int x, int y);
	void				StartDragSelect(int x, int y);
	void				ContDragSelect(int x, int y);
	void				StopDragSelect(int x, int y);

protected:
	void				ReadText(void);
	STextStyle*			MakeStyles(size uiFrom = 0);
	void				MakeFontChanges(CArrayTextFontChange* pasChanges, Ptr<CFont> pDefaultFont, int16 iDefaultBaselineShift, size uiFrom = 0);
	bool				HasDecorations(void);
	void				LayoutText(void);
	void				UpdateText(void);
	void				UpdateSize(void);
	int32				GetWrapWidth(void);
	void				FindStopStarts(void);
	void				FindLineStarts(void);
	size				FindGap(size uiLine);
	STextLayoutGap*		GetGap(size uiLine);
	bool				IsGapKept(STextLayoutGap* psGap);
	size				FindLastRow(size uiLine);
	void				GapsChanged(void);
	void				LinesInserted(size uiPosition, const char* szText, size uiLength);
	void				LinesRemoved(size uiStart, size uiEnd);

	//The colours under the text.
	void				UpdateBackgrounds(void);
	void				AddLineBackgroundRectangles(STextAreaBackground* psBackground);
	void				AddBoundaryRectangle(STextAreaBackground* psBackground);
	void				AddRangeRectangles(Ptr<CGeometryMap> pMap, size uiStart, size uiEnd, ARGB32 sColour);
	bool				GetLineRows(size uiLine, int32* piTop, int32* piBottom);
	int32				GetFullWidth(void);
	void				BackgroundsInserted(size uiPosition, size uiLength);
	void				BackgroundsRemoved(size uiStart, size uiEnd);
	void				BackgroundLinesInserted(size uiLine, bool bLineStart, size uiNewLines);
	void				BackgroundLinesRemoved(size uiFirst, size uiLast, bool bWhole);

	//The text as tiles.
	bool				UseTiles(void);
	void				ReadTiles(void);
	bool				CanAppendTiles(void);
	bool				CanAppendSprites(void);
	void				AppendSprites(size uiFrom);
	Ptr<CText>			ReadTextFrom(size uiFrom);
	void				AppendTiles(size uiFrom);
	void				AppendStops(size uiFrom);
	void				AppendLineStarts(size uiFrom);
	void				LayoutTiles(void);
	void				MakeTileSpans(STextStyle* psStyles, CArrayTileTextSpan* pasSpans);
	void				DecorateTiles(STextStyle* psStyles);
	void				DecorateTileStretch(uint8 uiDecoration, STextStyle* psStyles, size uiRow, size uiFirstStop, size uiEndStop);
	STextStyle*			GetTileStyle(STextStyle* psStyles, size uiStop);

	//The lines and stops the text is laid out in, by mpLayout or as tiles.
	size				NumLayoutLines(void);
	size				NumLayoutStops(void);
	STextLayoutLine*	GetLayoutLine(size uiLine);
	STextLayoutStop*	GetLayoutStop(size uiStop);

	//Characters and lines of the text.
	size				CharacterLength(size uiPosition);
	size				NextPosition(size uiPosition);
	size				PreviousPosition(size uiPosition);
	size				FindLineStart(size uiPosition);
	size				FindLineEnd(size uiPosition);
	size				FindLineIndent(size uiLineStart);
	size				CountColumns(size uiLineStart, size uiPosition);
	size				FindColumn(size uiLineStart, size uiColumn);
	size				FindWordLeft(size uiPosition);
	size				FindWordRight(size uiPosition);
	bool				IsWhitespace(char c);
	bool				IsWordChar(char c);

	//The rows the text is laid out in.
	bool				HasStops(void);
	size				FindStop(size uiPosition);
	bool				IsRowWrapped(size uiRow);
	size				FindRow(size uiPosition, bool bRowEnd);
	size				GetRowStart(size uiRow);
	size				GetRowEnd(size uiRow, bool* pbRowEnd);
	int32				GetRowRight(size uiRow);
	size				FindRowAt(int32 y);
	size				FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd);
	int32				GetPositionX(size uiPosition, bool bRowEnd);
	int32				GetCaretX(void);
	size				GetCaretRow(void);
	int32				GetPageHeight(void);
	int32				GetNewLineWidth(void);

	//Moving the caret.
	bool				IsSelected(void);
	void				MoveCaret(size uiPosition, bool bSelect, bool bRowEnd = false, bool bKeepUpDown = false);
	void				MoveUpDown(ETextAreaMove eMove, bool bSelect);
	size				ScrollPage(size uiRow, bool bDown);
	void				MoveHome(bool bSelect);
	void				MoveEnd(bool bSelect);
	void				ScrollBy(int32 iDown);

	//Changing the text.  All changes go through InsertText and RemoveText.
	void				InsertText(size uiPosition, const char* szText);
	void				RemoveText(size uiStart, size uiEnd);
	void				ReplaceSelection(const char* szText);
	bool				DeleteSelection(void);
	void				IndentLines(size uiStart, size uiEnd);
	void				OutdentLines(size uiStart, size uiEnd);
	void				TextChanged(void);
	void				KeepEdit(void);
	void				StartEdit(void);
	void				BeginEdit(void);
	void				UndoChanged(ETextAreaUndo eChange, size uiIdBefore, size uiIdAfter);
	bool				LoadUndo(size uiIndex);
	void				ToClipboard(void (CTextArea::*fTake)(CChars* pszDest));

	void				ShowCaret(void);
	void				UpdateMarks(void);
};


#endif // __TEXT_AREA_H__


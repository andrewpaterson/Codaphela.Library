#ifndef __TEXT_AREA_H__
#define __TEXT_AREA_H__
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
#include "Canvas.h"
#include "ComponentInputEvent.h"


//A range of the text in a style.  uiStart and uiLength are in bytes of the UTF8 text.
struct STextAreaSpan
{
	size				uiStart;
	size				uiLength;
	STextStyleOverride	sOverride;
};


typedef CArrayTemplate<STextAreaSpan>	CArrayTextAreaSpan;
typedef CArrayTemplate<size>			CArrayTextAreaStopStart;


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
	Ptr<CText>			mpText;
	Ptr<CTextLayout>	mpLayout;
	Ptr<CSpriteMap>		mpSpriteMap;
	Ptr<CGeometryMap>	mpGeometryMap;  //The lines drawn along the text, over it.
	Ptr<CMapFontDraw>	mpFontDraw;
	Ptr<CMaps>			mpMaps;
	bool				mbWordWrap;
	SInt32Vec2			msTextSize;

	Ptr<CGeometryMap>	mpSelectionMap;  //Under the text.
	Ptr<CGeometryMap>	mpCaretMap;  //Over the text.
	ARGB32				msSelectionColour;
	ARGB32				msCaretColour;
	CArrayTextAreaStopStart	mauiStopStarts;  //The byte each of the layout's stops is before.  Carriage returns have none.
	size				muiCaret;
	size				muiAnchor;  //Where the selection started, MAX_SIZE for no selection.
	bool				mbCaretAtRowEnd;  //A caret at the start of a wrapped row is drawn at the end of the row above.
	int32				miUpDownX;  //Where up and down try to keep the caret, -1 until they are used.
	int32				miTabWidth;  //In spaces.
	bool				mbEditable;
	bool				mbFocussed;
	bool				mbDragging;

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
	void				SetFont(Ptr<CFont> pWhiteFont);
	Ptr<CFont>			GetFont(void);
	void				SetFontColours(Ptr<CFontColours> pColours);
	Ptr<CFontColours>	GetFontColours(void);
	void				SetFontFamily(Ptr<CFontFamily> pFamily);
	Ptr<CFontFamily>	GetFontFamily(void);
	void				SetWordWrap(bool bWordWrap);
	bool				IsWordWrap(void);

	void				SetStyle(STextStyle* psStyle);
	void				GetStyle(STextStyle* psStyle);
	void				SetColour(ARGB32 sColour);
	ARGB32				GetColour(void);
	bool				AddSpan(size uiStart, size uiLength, ARGB32 sColour);
	bool				AddSpan(size uiStart, size uiLength, STextStyleOverride* psOverride);
	void				ClearSpans(void);
	size				NumSpans(void);
	void				GetStyleAt(size uiPosition, STextStyle* psStyle);
	ARGB32				GetColourAt(size uiPosition);

	void				SetEditable(bool bEditable);
	bool				IsEditable(void);
	void				SetTabWidth(int32 iTabWidth);
	void				SetMarkColours(ARGB32 sSelection, ARGB32 sCaret);
	size				GetCaretPosition(void);
	void				SetCaretPosition(size uiPosition);
	bool				GetSelection(size* puiStart, size* puiEnd);
	void				Select(size uiStart, size uiEnd);
	size				FindPositionAt(int x, int y);
	void				GetCaretBounds(CInt32Rect* pcDest);

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
	void				Space(bool bInsert);
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
	void				CutLine(CChars* pszDest);
	void				Duplicate(void);
	void				DeleteWordRemainingRight(void);
	void				DeleteWordRemainingLeft(void);
	void				DeleteLine(void);

	void				LineUp(void);
	void				LineDown(void);

	SInt32Vec2			GetTextSize(void);
	SInt32Vec2			GetRequiredSize(void);
	SInt32Vec2			GetDesiredSize(void) override;

	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool				Draw(void) override;
	bool				WantsEnter(void) override;

	void				FocusGained(void) override;
	void				FocusLost(void) override;
	void				CommandAction(SComponentInputEvent* psEvent);
	void				PrintableAction(SComponentInputEvent* psEvent);
	void				ActivateComponent(int x, int y);
	void				ExecuteComponent(int x, int y);
	void				StartDragSelect(int x, int y);
	void				ContDragSelect(int x, int y);
	void				StopDragSelect(int x, int y);

protected:
	void				ReadText(void);
	void				MakeFontChanges(CArrayTextFontChange* pasChanges, Ptr<CFont> pDefaultFont, int16 iDefaultBaselineShift);
	void				LayoutText(void);
	void				UpdateText(void);
	void				UpdateSize(void);
	int32				GetWrapWidth(void);
	void				FindStopStarts(void);

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
	void				MoveUpDown(bool bDown, bool bPage, bool bSelect);
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

	void				ShowCaret(void);
	void				UpdateMarks(void);
};


#endif // __TEXT_AREA_H__


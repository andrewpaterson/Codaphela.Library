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
#include "BaseLib/ArrayElementNotFound.h"
#include "BaseLib/Timer.h"
#include "BaseLib/UTF8.h"
#include "StandardLib/ClassDefines.h"
#include "MapsCanvasDraw.h"
#include "Window.h"
#include "NativeWindowFactory.h"
#include "ComponentInput.h"
#include "TextArea.h"


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  Line feeds start new lines.  pWhiteFont's image must
// be white (see CFontColours); the text area makes its own colours of it.
// It is the only style and size the text area has.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pWhiteFont, const char* szText)
{
	Init(pWindow, pTheme, OMalloc<CFontColours>(pWhiteFont), szText);
}


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  pColours can be shared by text areas so each colour is
// only made once.  It is the only style and size the text area has.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFontColours> pColours, const char* szText)
{
	Init(pWindow, pTheme, OMalloc<CFontFamily>(pColours), szText);
}


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  Line feeds start new lines.  pFamily can be shared by
// text areas.  The caret starts at the start of the text, and the text
// area is editable.  The selection and caret are the theme's colours.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFontFamily> pFamily, const char* szText)
{
	Ptr<CMapsCanvasDraw>	pDraw;

	PreInit();

	mcDocument.Init(szText);
	mpLayout = OMalloc<CTextAreaLayout>(&mcDocument, pFamily);
	mpBackgroundMap = OMalloc<CGeometryMap>();
	mcBackgrounds.Init();
	mcShapes.Init(&mpLayout, &mcDocument);
	mbBackgroundsDirty = false;
	miBackgroundWidth = 0;
	mpSelectionMap = OMalloc<CGeometryMap>();
	mbRowsChanged = false;
	mcBands.Init();
	mpMaps = OMalloc<CMaps>();
	mpMaps->AddMap(mpBackgroundMap);
	mpMaps->AddMap(mpSelectionMap);
	mpMaps->AddMap(mpLayout->GetSpriteMap());
	mpMaps->AddMap(mpLayout->GetTileMap());
	mpMaps->AddMap(mpLayout->GetGeometryMap());
	mpMaps->SetViewportPosition(0, 0);
	msSelectionColour = pTheme->GetColour(TC_TextSelection);
	msCaretColour = pTheme->GetColour(TC_Caret);
	mbCaretLine = false;
	msCaretLineColour = 0;
	muiCaretLine = 0;
	mcOccurrences.Init(&mcDocument);
	msOccurrenceColour = pTheme->GetColour(TC_TextOccurrence);
	msWriteOccurrenceColour = pTheme->GetColour(TC_TextWriteOccurrence);
	mcDocument.AddListener(&mcBackgrounds);
	mcEditing.Init(&mcDocument, this);
	mcUndoEvent.Init();
	mcChanged.Init();
	mbSelectable = true;

	//The text is drawn over whatever is behind the text area.
	pDraw = OMalloc<CMapsCanvasDraw>(mpMaps);
	CCanvas::Init(pWindow, pDraw);
	SetTransparent(true);
	SetCanGetFocus(true);

	Relayout();

	mcInput.Init(this, &mcEditing, true);
	mcInput.SetTextDragEnabled(true);
	AddInputAction(CIC_LineUp, this, &CTextArea::LineUp);
	AddInputAction(CIC_LineDown, this, &CTextArea::LineDown);

	//The undo history starts with the text it was made with.
	ClearUndo();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Free(void)
{
	if (mpWindow.IsNotNull())
	{
		mpWindow->CancelPosted(this);
	}
	mcBands.Kill();
	mcShapes.Kill();
	mcBackgrounds.Kill();
	mcChanged.Kill();
	mcUndoEvent.Kill();
	mcInput.Kill();
	mcEditing.Kill();
	mcOccurrences.Kill();
	mcDocument.Kill();
	CCanvas::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Class(void)
{
	CCanvas::Class();

	U_Unknown(CTextDocument, mcDocument);
	U_Unknown(CTextAreaUndoEvent, mcUndoEvent);
	U_Unknown(CComponentEvent<CTextArea>, mcChanged);
	M_Pointer(mpLayout);
	M_Pointer(mpMaps);
	M_Pointer(mpBackgroundMap);
	U_Unknown(CTextBackgrounds, mcBackgrounds);
	U_Unknown(CTextAreaShapes, mcShapes);
	U_Bool(mbBackgroundsDirty);
	U_Int32(miBackgroundWidth);
	M_Pointer(mpSelectionMap);
	U_Bool(mbRowsChanged);
	U_Unknown(CTextAreaBands, mcBands);
	U_UInt32(msSelectionColour);
	U_UInt32(msCaretColour);
	U_Bool(mbCaretLine);
	U_UInt32(msCaretLineColour);
	U_Size(muiCaretLine);
	U_Unknown(CTextAreaOccurrences, mcOccurrences);
	U_UInt32(msOccurrenceColour);
	U_UInt32(msWriteOccurrenceColour);
	U_Unknown(CTextEditing, mcEditing);
	U_Bool(mbSelectable);
	U_Unknown(CTextEditingInput, mcInput);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The spans, backgrounds and line gaps are removed; they were of the old
// text.  The caret goes to the start and nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetText(const char* szText)
{
	mcDocument.SetText(szText);
	mcOccurrences.TextSet();
	mcBackgrounds.Clear();
	mcEditing.Reset();
	mpLayout->TextSet();
	LaidOut();
	ClearUndo();

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds szText (UTF8) to the end of the text in sColour.  Only what was
// added (and the line it joins) is read and laid out, as tiles or as
// sprites, so appending to a long text, an output pane say, costs what is
// added, not the whole text.  The text area is laid out again: it is bigger.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::AppendText(const char* szText, ARGB32 sColour)
{
	if ((szText == NULL) || (szText[0] == '\0'))
	{
		return;
	}
	mpLayout->Append(szText, sColour);
	mcOccurrences.TextSet();
	LaidOut();

	//What it is laid out from has changed.
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// pWhiteFont's image must be white (see CFontColours).  It becomes the only
// style and size the text area has.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetFont(Ptr<CFont> pWhiteFont)
{
	SetFontColours(OMalloc<CFontColours>(pWhiteFont));

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetFontColours(Ptr<CFontColours> pColours)
{
	SetFontFamily(OMalloc<CFontFamily>(pColours));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetFontFamily(Ptr<CFontFamily> pFamily)
{
	mpLayout->SetFontFamily(pFamily);
	Relayout();

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetWordWrap(bool bWordWrap)
{
	if (mpLayout->SetWordWrap(bWordWrap))
	{
		Relayout();
	}

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// The style of text that isn't in a span, or where spans don't set a
// property.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetStyle(STextStyle* psStyle)
{
	mpLayout->SetStyle(psStyle);

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::GetStyle(STextStyle* psStyle) { *psStyle = *mpLayout->GetStyle(); }


//////////////////////////////////////////////////////////////////////////
//
// The colour of text that isn't in a span that sets a colour.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetColour(ARGB32 sColour)
{
	RowsChanged();
	mpLayout->SetColour(sColour);
}


//////////////////////////////////////////////////////////////////////////
//
// A span that only sets a colour.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::AddSpan(size uiStart, size uiLength, ARGB32 sColour)
{
	STextStyleOverride	sOverride;

	sOverride.Init();
	sOverride.SetColour(sColour);
	return AddSpan(uiStart, uiLength, &sOverride);
}


//////////////////////////////////////////////////////////////////////////
//
// uiStart and uiLength are in bytes of the UTF8 text.  A span that starts
// or ends inside a character starts or ends at the next character.
// Returns false if the span goes past the end of the text.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::AddSpan(size uiStart, size uiLength, STextStyleOverride* psOverride)
{
	RowsChanged();
	return mpLayout->AddSpan(uiStart, uiLength, psOverride);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ClearSpans(void)
{
	RowsChanged();
	mpLayout->ClearSpans();
}


//////////////////////////////////////////////////////////////////////////
//
// Colours bytes uiStart to uiEnd of the UTF8 text, under the selection.
// False if the range isn't in the text; an empty range is ignored.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::AddBackground(size uiStart, size uiEnd, ARGB32 sColour)
{
	if ((uiStart > uiEnd) || (uiEnd > mcDocument.Length()))
	{
		return false;
	}
	if (uiStart == uiEnd)
	{
		return true;
	}

	mcBackgrounds.Add(TAB_Range, uiStart, uiEnd, 0, sColour);
	BackgroundsChanged();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Colours lines uiStartLine to uiEndLine across the text area's whole
// width, not the gaps below them.  False if they aren't lines of the text;
// no lines are ignored.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::AddLineBackground(size uiStartLine, size uiEndLine, ARGB32 sColour)
{
	if ((uiStartLine > uiEndLine) || (uiEndLine > NumLines()))
	{
		return false;
	}
	if (uiStartLine == uiEndLine)
	{
		return true;
	}

	mcBackgrounds.Add(TAB_Lines, uiStartLine, uiEndLine, 0, sColour);
	BackgroundsChanged();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// pcListener is told each edit of the text (CTextDocumentListener), to
// keep what it has anchored to the text in step, until it is removed.  It
// isn't held.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::AddDocumentListener(CTextDocumentListener* pcListener) { mcDocument.AddListener(pcListener); }
void CTextArea::RemoveDocumentListener(CTextDocumentListener* pcListener) { mcDocument.RemoveListener(pcListener); }


//////////////////////////////////////////////////////////////////////////
//
// A line iThickness pixels high across the text area's whole width,
// centred on the top of line uiLine, or on the bottom of the last line
// (below its gap) if uiLine is NumLines.  It stays above its line as the
// text is edited.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::AddLineBoundary(size uiLine, int32 iThickness, ARGB32 sColour)
{
	if ((uiLine > NumLines()) || (iThickness <= 0))
	{
		return false;
	}

	mcBackgrounds.Add(TAB_Boundary, uiLine, uiLine, iThickness, sColour);
	BackgroundsChanged();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ClearBackgrounds(void)
{
	if (mcBackgrounds.Clear())
	{
		BackgroundsChanged();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::NumBackgrounds(void)
{
	return mcBackgrounds.NumBackgrounds();
}


//////////////////////////////////////////////////////////////////////////
//
// In the order they were added, as they are now the text has been edited.
//
//////////////////////////////////////////////////////////////////////////
STextAreaBackground* CTextArea::GetBackground(size uiIndex)
{
	return mcBackgrounds.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CTextArea::GetColourAt(size uiPosition)
{
	STextStyle	sStyle;

	GetStyleAt(uiPosition, &sStyle);
	return sStyle.sColour;
}


//////////////////////////////////////////////////////////////////////////
//
// The text is laid out again for the width the text area is given, and
// read again if its spans or style changed.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2	sTextSize;

	CCanvas::Layout(sPosition, sAreaSize);

	mpLayout->SetWidth(mcActualBounds.GetWidth());
	if (mpLayout->IsTextDirty() || mpLayout->IsWrapWidthChanged())
	{
		//Wrapped to a new width, or in new styles, the text can be a different size so the window must lay its
		//components out again.
		sTextSize = mpLayout->GetTextSize();
		if (mpLayout->IsTextDirty())
		{
			mpLayout->ReadText();
		}
		mpLayout->LayoutText();
		LaidOut();
		if (sTextSize != mpLayout->GetTextSize())
		{
			mpWindow->LayoutAgain();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Spans or a style set since the text area was last laid out are drawn,
// and the backgrounds as they are now.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::Draw(void)
{
	UpdateText();
	if (mbBackgroundsDirty || (miBackgroundWidth != GetFullWidth()))
	{
		UpdateBackgrounds();
	}
	return CCanvas::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
// The text area's setters only note that its text, spans, backgrounds or
// selection changed.  Here, as the window is painted, they are brought up
// to date and the bands of what will be drawn compared with those of what
// was (CTextAreaBands): only the rows that changed are drawn again.  All of
// it is drawn when there are no bands to compare (no rows, nothing drawn
// yet), when it scrolled, moved or was invalidated, or when what is drawn
// across its width changed.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::PrepareArea(bool bDrawn, bool bClipped, CInt32Rect* pcClip, CInt32Rect* pcArea)
{
	CTextAreaBands	cBands;
	CArrayInt32Vec2	asChanged;
	CInt32Rect		cPart;
	SInt32Vec2		sTextSize;
	SInt32Vec2*		psChanged;
	int32			iWidth;
	bool			bAll;
	size			i;

	CCanvas::PrepareArea(bDrawn, bClipped, pcClip, pcArea);
	if ((pcArea->GetWidth() <= 0) || (pcArea->GetHeight() <= 0))
	{
		mcBands.Clear();
		return;
	}
	if (!mbRowsChanged && !mpLayout->IsTextDirty() && !mbBackgroundsDirty && (miBackgroundWidth == GetFullWidth()) && !IsContentDirty())
	{
		return;
	}

	sTextSize = mpLayout->GetTextSize();
	UpdateText();
	bAll = IsContentDirty() || !mcBands.IsComplete() || (miBackgroundWidth != GetFullWidth());
	if (sTextSize != mpLayout->GetTextSize())
	{
		//Its new size is laid out at the next paint.
		LayoutChanged();
	}
	if (mbBackgroundsDirty || (miBackgroundWidth != GetFullWidth()))
	{
		UpdateBackgrounds();
	}
	mbRowsChanged = false;

	cBands.Init();
	if (!SummariseBands(&cBands))
	{
		bAll = true;
	}
	if (bAll)
	{
		MarkContentChanged();
		mpWindow->AddDamage(pcArea);
	}
	else
	{
		asChanged.Init();
		cBands.Compare(&mcBands, &asChanged);
		iWidth = GetFullWidth();
		for (i = 0; i < asChanged.NumElements(); i++)
		{
			psChanged = asChanged.Get(i);
			cPart.Init(0, psChanged->x, iWidth, psChanged->y);
			DamagePart(&cPart);
		}
		asChanged.Kill();
	}
	mcBands.Take(&cBands);
	cBands.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The bands (CTextAreaBands) of the part of the text area that shows, as
// it will be drawn: one for each row from its top to the next's (the last
// to the bottom of the text area), each a hash of the row's text, its
// styles and the shapes crossing the band (backgrounds, selection, lines
// along the text).  False without rows.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::SummariseBands(CTextAreaBands* pcBands)
{
	SInt32Vec2	sOrigin;
	int32		iTop;
	int32		iBottom;
	int32		iEnd;

	if (!HasRows())
	{
		return false;
	}

	sOrigin = GetNativeOrigin();
	iTop = sOrigin.y;
	iBottom = sOrigin.y + GetNativeSize().y;
	iEnd = (mcActualBounds.GetHeight() > mpLayout->GetTextSize().y) ? mcActualBounds.GetHeight() : mpLayout->GetTextSize().y;
	pcBands->AddRows(&mpLayout, &mcDocument, iTop, iBottom, iEnd);

	//The shapes drawn under and over the text, in the order they are drawn.
	pcBands->MixShapes(mpBackgroundMap, iTop, iBottom);
	pcBands->MixShapes(mpSelectionMap, iTop, iBottom);
	pcBands->MixShapes(mpLayout->GetGeometryMap(), iTop, iBottom);
	pcBands->Complete();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The height of the lines as they were last laid out.  A word wrapped text
// area has no required width (-1), it takes whatever width it is given.
// Otherwise the width is the width of the text from the text layout.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTextArea::GetRequiredSize(void)
{
	SInt32Vec2	sSize;

	UpdateSize();
	sSize = mpLayout->GetTextSize();
	if (mpLayout->IsWordWrap())
	{
		sSize.x = -1;
	}
	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
// A desired size overrides the required size on each axis it is set on.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTextArea::GetDesiredSize(void)
{
	SInt32Vec2	sSize;

	sSize = GetRequiredSize();
	if (msDesiredSize.x >= 0)
	{
		sSize.x = msDesiredSize.x;
	}
	if (msDesiredSize.y >= 0)
	{
		sSize.y = msDesiredSize.y;
	}
	return sSize;
}


//////////////////////////////////////////////////////////////////////////
//
// The width of the widest line and the height of all the lines as they
// were last laid out.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTextArea::GetTextSize(void)
{
	UpdateSize();
	return mpLayout->GetTextSize();
}


char* CTextArea::GetText(void) { return mcDocument.Text(); }
size CTextArea::GetChangeCount(void) { return mcDocument.GetChangeCount(); }  //Each edit counts one more, so the text's owner can tell it changed since it last looked.
Ptr<CFontFamily> CTextArea::GetFontFamily(void) { return mpLayout->GetFontFamily(); }
bool CTextArea::IsWordWrap(void) { return mpLayout->IsWordWrap(); }
ARGB32 CTextArea::GetColour(void) { return mpLayout->GetStyle()->sColour; }
Ptr<CFontColours> CTextArea::GetFontColours(void) { return mpLayout->GetFontColours(); }  //The colours of the text area's style's face and size.
Ptr<CFont> CTextArea::GetFont(void) { return mpLayout->GetFont(); }  //The white font of the text area's style's face and size.
int32 CTextArea::GetTabPixels(void) { return mpLayout->GetTabPixels(); }
void CTextArea::GetStyleAt(size uiPosition, STextStyle* psStyle) { mpLayout->GetStyleAt(uiPosition, psStyle); }
size CTextArea::NumSpans(void) { return mpLayout->NumSpans(); }


//////////////////////////////////////////////////////////////////////////
//
// A text area that isn't editable can still be navigated, selected and
// copied from.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetEditable(bool bEditable)
{
	mcEditing.SetEditable(bEditable);
}


//////////////////////////////////////////////////////////////////////////
//
// A text area that isn't selectable passes the pointer's presses and
// drags to its parent, which can act on them; give it focus not either
// (SetCanGetFocus) and it is only something to read.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetSelectable(bool bSelectable)
{
	mbSelectable = bSelectable;
	mcInput.SetPointerEnabled(bSelectable);
}


//////////////////////////////////////////////////////////////////////////
//
// How many spaces apart the tab stops are.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetTabWidth(int32 iTabWidth)
{
	mcEditing.SetTabWidth(iTabWidth < 1 ? 1 : (size)iTabWidth);
	mpLayout->SetTabWidth(mcEditing.GetTabWidth());

	//Tabs go to tab stops, as tiles or as sprites, so the text is laid out again.
	if (mpLayout->IsTiled())
	{
		mpLayout->ReadText();
	}
	mpLayout->LayoutText();
	LaidOut();

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// The selection is drawn under the text, the caret over it.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetMarkColours(ARGB32 sSelection, ARGB32 sCaret)
{
	msSelectionColour = sSelection;
	msCaretColour = sCaret;
	UpdateMarks();
}


//////////////////////////////////////////////////////////////////////////
//
// The line the caret is on is highlighted in sColour, under the other
// backgrounds and the selection, as IntelliJ's caret row is, with or
// without the keys; or not, without bShown.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetCaretLine(bool bShown, ARGB32 sColour)
{
	mbCaretLine = bShown;
	msCaretLineColour = sColour;
	muiCaretLine = FindLine(GetCaretPosition());
	BackgroundsChanged();
}


bool CTextArea::IsCaretLineShown(void) { return mbCaretLine; }
size CTextArea::GetCaretLine(void) { return muiCaretLine; }


//////////////////////////////////////////////////////////////////////////
//
// uiPosition is a byte of the text and should be the start of a
// character.  Nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetCaretPosition(size uiPosition)
{
	mcEditing.SetCaretPosition(uiPosition);

	//The caret is shown where the window lays the text area out; the marks redraw what of it they change.
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if nothing is selected, then the start and end are the
// caret.  The start is before the end.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::GetSelection(size* puiStart, size* puiEnd) { return mcEditing.GetSelection(puiStart, puiEnd); }


//////////////////////////////////////////////////////////////////////////
//
// Selects from uiStart to uiEnd with the caret at uiEnd.  The same start
// and end selects nothing.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Select(size uiStart, size uiEnd)
{
	mcEditing.Select(uiStart, uiEnd);

	//The caret is shown where the window lays the text area out; the marks redraw what of it they change.
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are relative to the text area's top left.  The gap between
// characters nearest to them on the row they are in, or the nearest row.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindPositionAt(int x, int y)
{
	bool	bRowEnd;

	UpdateText();
	if (!HasRows())
	{
		return 0;
	}
	return FindPositionInRow(FindRowAt(y), x, &bRowEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// The character x, y (relative to the text area) is over, not the gap
// nearest it.  Returns false past its row's end, with the gap there.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::FindCharacterAt(int x, int y, size* puiPosition)
{
	UpdateText();
	return mcEditing.FindCharacterAt(x, y, puiPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// A press beside the text (x, y relative to the text area, as a code
// editor's gutter press is) selects the line of the row y is in and drags
// by whole lines from it; extending, from the selection's anchor.  The
// text area must have the focus.  Returns false if the text can't be
// selected.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::StartLineDrag(int x, int y, bool bExtend)
{
	if (!mbSelectable)
	{
		return false;
	}
	UpdateText();
	mcInput.StartLineDrag(x, y, bExtend);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The caret's pixels relative to the text area's top left, as it would be
// drawn.  It is a pixel wide and as high as its row.  While the selected
// text is dragged it is the drop caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::GetCaretBounds(CInt32Rect* pcDest)
{
	STextLayoutLine*	psLine;
	int32				x;
	size				uiPosition;
	bool				bRowEnd;

	UpdateText();
	if (!HasRows())
	{
		pcDest->Init(0, 0, 1, mpLayout->GetTextSize().y);
		return;
	}

	//Dragging the selected text, the caret is drawn where it would be dropped.
	if (mcInput.GetDropPosition(&uiPosition, &bRowEnd))
	{
		psLine = mpLayout->GetLayoutLine(FindRow(uiPosition, bRowEnd));
		x = GetPositionX(uiPosition, bRowEnd);
	}
	else
	{
		psLine = mpLayout->GetLayoutLine(mcEditing.GetCaretRow());
		x = mcEditing.GetCaretX();
	}
	if ((mcActualBounds.GetWidth() > 0) && (x > mcActualBounds.GetWidth() - 1))
	{
		x = mcActualBounds.GetWidth() - 1;
	}
	pcDest->Init(x, psLine->iTop, x + 1, psLine->iTop + psLine->iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// The lines are ended by line feeds; there is always at least one.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::NumLines(void) { return mcDocument.NumLines(); }


//////////////////////////////////////////////////////////////////////////
//
// The first byte of line uiLine, or the end of the text past the last.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::GetLineStart(size uiLine) { return mcDocument.GetLineStart(uiLine); }


//////////////////////////////////////////////////////////////////////////
//
// Where line uiLine ends: its line feed, or the end of the text for the
// last line and any past it.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::GetLineEnd(size uiLine) { return mcDocument.GetLineEnd(uiLine); }


//////////////////////////////////////////////////////////////////////////
//
// The line uiPosition (a byte of the text) is on.  A line feed is on the
// line it ends.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindLine(size uiPosition) { return mcDocument.FindLine(uiPosition); }


//////////////////////////////////////////////////////////////////////////
//
// The pixels of line uiLine's rows, relative to the text area's top left,
// as wide as the text area or its text, whichever is wider.  Not its gap.
// False if there is no such line or the text isn't laid out.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::GetLineBounds(size uiLine, CInt32Rect* pcDest)
{
	STextLayoutLine*	psFirst;
	STextLayoutLine*	psLast;
	int32				iWidth;

	UpdateText();
	if ((uiLine >= NumLines()) || !HasRows())
	{
		return false;
	}

	psFirst = mpLayout->GetLayoutLine(mpLayout->GetLayoutStop(mpLayout->FindStop(GetLineStart(uiLine)))->uiLine);
	psLast = mpLayout->GetLayoutLine(mpLayout->FindLastRow(uiLine));
	iWidth = GetFullWidth();
	pcDest->Init(0, psFirst->iTop, iWidth, psLast->iTop + psLast->iHeight);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Leaves iHeight pixels empty below line uiLine, under its last row (see
// GapsChanged for when).  0 removes the gap.  False if there is no such
// line.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::SetLineGap(size uiLine, int32 iHeight)
{
	if (uiLine >= NumLines())
	{
		return false;
	}
	if (mpLayout->GetGaps()->Set(uiLine, iHeight))
	{
		GapsChanged();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The id of line uiLine's gap, 0 if it has none.
//
//////////////////////////////////////////////////////////////////////////
uint32 CTextArea::GetLineGapId(size uiLine)
{
	return mpLayout->GetGaps()->GetId(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
// The line the gap with id uiId is below, MAX_SIZE if it has gone.  It
// looks through every gap; to find many, go through GetLineGaps once.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindLineGap(uint32 uiId)
{
	return mpLayout->GetGaps()->FindLine(uiId);
}


//////////////////////////////////////////////////////////////////////////
//
// The gaps, sorted by line.  Don't change them.
//
//////////////////////////////////////////////////////////////////////////
CArrayTextLayoutGap* CTextArea::GetLineGaps(void)
{
	return mpLayout->GetGaps()->GetGaps();
}


//////////////////////////////////////////////////////////////////////////
//
// The gaps become a copy of pasGaps (sorted by line, at most one a line,
// with their ids), such as another text area's, laid out again only if
// their lines or heights changed.  Gaps past the last line, or of no
// height, are left out.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetLineGaps(CArrayTextLayoutGap* pasGaps)
{
	if (mpLayout->GetGaps()->Copy(pasGaps, NumLines()))
	{
		GapsChanged();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The height asked for below line uiLine (SetLineGap), 0 for none.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextArea::GetLineGap(size uiLine)
{
	return mpLayout->GetGaps()->GetHeight(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::NumLineGaps(void)
{
	return mpLayout->GetGaps()->NumGaps();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ClearLineGaps(void)
{
	if (mpLayout->GetGaps()->Clear())
	{
		GapsChanged();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pixels of line uiLine's gap as laid out, relative to the text area's
// top left and as wide as GetLineBounds.  In tiles it can be higher than
// was asked for.  False if the line has no gap or the text isn't laid out.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::GetLineGapBounds(size uiLine, CInt32Rect* pcDest)
{
	size				uiRow;
	STextLayoutLine*	psLine;
	int32				iTop;
	int32				iBottom;
	int32				iWidth;

	UpdateText();
	if ((GetLineGap(uiLine) <= 0) || (uiLine >= NumLines()) || !HasRows())
	{
		return false;
	}

	uiRow = mpLayout->FindLastRow(uiLine);
	psLine = mpLayout->GetLayoutLine(uiRow);
	iTop = psLine->iTop + psLine->iHeight;
	iBottom = (uiRow + 1 < mpLayout->NumLayoutLines()) ? mpLayout->GetLayoutLine(uiRow + 1)->iTop : mpLayout->GetTextSize().y;
	iWidth = GetFullWidth();
	pcDest->Init(0, iTop, iWidth, iBottom);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The moves and edits of the text area's editing (see CTextEditing).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SelectAll(void) { Select(0, mcDocument.Length()); }
bool CTextArea::ReplaceRanges(STextRange* asRanges, size uiNumRanges, const char* szText) { return mcEditing.ReplaceRanges(asRanges, uiNumRanges, szText); }


//////////////////////////////////////////////////////////////////////////
//
// The selection grows to the smallest range around it that is bigger, or
// goes back to what it grew from (see CTextEditing::ExtendSelection).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ExtendSelection(void)
{
	mcEditing.ExtendSelection();

	//The caret is shown where the window lays the text area out; the marks redraw what of it they change.
	LayoutChanged();
}


void CTextArea::ShrinkSelection(void)
{
	mcEditing.ShrinkSelection();
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// pcRanges adds ranges Extend Selection can grow to, a parse's syntax
// say; NULL for none.  It isn't held.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetSelectionRanges(CTextSelectionRanges* pcRanges)
{
	mcEditing.SetSelectionRanges(pcRanges);
}


//////////////////////////////////////////////////////////////////////////
//
// A box (column) selection from row uiAnchorRow at x iAnchorX to row
// uiCaretRow at x iCaretX, relative to the text area (see
// CTextEditing::BoxSelect).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::BoxSelect(size uiAnchorRow, int32 iAnchorX, size uiCaretRow, int32 iCaretX)
{
	UpdateText();
	mcEditing.BoxSelect(uiAnchorRow, iAnchorX, uiCaretRow, iCaretX);
	LayoutChanged();
}


bool CTextArea::IsBox(void) { return mcEditing.IsBox(); }
void CTextArea::GetBoxRanges(CArrayTextRange* pasDest) { mcEditing.GetBoxRanges(pasDest); }


//////////////////////////////////////////////////////////////////////////
//
// Scrolls a line without moving the caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::LineUp(void)
{
	Ptr<CFont>	pFont;

	pFont = GetFont();
	ScrollBy(pFont.IsNotNull() ? -pFont->GetLineHeight() : -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::LineDown(void)
{
	Ptr<CFont>	pFont;

	pFont = GetFont();
	ScrollBy(pFont.IsNotNull() ? pFont->GetLineHeight() : 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Enter is a line feed, it doesn't click the window's default button.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::WantsEnter(void)
{
	return mcEditing.IsEditable();
}


const char* CTextArea::GetKeyContext(void) { return COMPONENT_INPUT_EDITOR_CONTEXT; }  //The keys an application has bound in the Editor context, outside of a dialog.
bool CTextArea::TakesTyping(void) { return true; }


//////////////////////////////////////////////////////////////////////////
//
// Undo and redo only with something to undo or redo; cut and paste only
// in editable text; paste from history only with something to choose
// with, too.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::CanDoCommand(EComponentInputCommand eCommand)
{
	switch (eCommand)
	{
	case CIC_Undo:
		return CanUndo();
	case CIC_Redo:
		return CanRedo();
	case CIC_Cut:
	case CIC_Paste:
		return IsEditable() && CComponent::CanDoCommand(eCommand);
	case CIC_PasteFromHistory:
		return mcInput.CanPasteFromHistory();
	default:
		return CComponent::CanDoCommand(eCommand);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A context menu opened from the keys opens under the caret, as Visual
// Studio's does, kept in the part of the text that is seen.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTextArea::GetContextMenuPosition(void)
{
	CInt32Rect	cCaret;
	CInt32Rect	cView;
	SInt32Vec2	sPosition;

	GetCaretBounds(&cCaret);
	sPosition = GetPosition();
	sPosition.x += cCaret.GetLeft();
	sPosition.y += cCaret.GetBottom();
	if (GetScrollView(&cView))
	{
		if (sPosition.x < cView.GetLeft())
		{
			sPosition.x = cView.GetLeft();
		}
		else if (sPosition.x > cView.GetRight())
		{
			sPosition.x = cView.GetRight();
		}
		if (sPosition.y < cView.GetTop())
		{
			sPosition.y = cView.GetTop();
		}
		else if (sPosition.y > cView.GetBottom())
		{
			sPosition.y = cView.GetBottom();
		}
	}
	return sPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::FocusGained(void)
{
	UpdateMarks();
}


//////////////////////////////////////////////////////////////////////////
//
// Losing focus, as when the window is deactivated, ends a drag.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::FocusLost(void)
{
	mcInput.FocusLost();
	UpdateMarks();
}


//////////////////////////////////////////////////////////////////////////
//
// The clipboard is the window's, the system's.  Cutting or copying
// nothing leaves it as it was.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::CutToClipboard(void) { mcInput.CutToClipboard(); }
void CTextArea::CopyToClipboard(void) { mcInput.CopyToClipboard(); }
void CTextArea::PasteFromClipboard(void) { mcInput.PasteFromClipboard(); }


//////////////////////////////////////////////////////////////////////////
//
// Like spans, the gaps are laid out when the text area is next laid out,
// drawn, asked its size or asked where something is, so changing many is
// laid out once.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::GapsChanged(void)
{
	mpLayout->GapsChanged();

	//The rows below the gaps move: what of the text area that changes is found when it is next drawn.
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// The height of the scroll pane's view, or of the text area if it isn't
// in one.  At least a line.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextArea::GetPageHeight(void)
{
	CInt32Rect	cView;
	int32		iHeight;
	Ptr<CFont>	pFont;

	if (GetScrollView(&cView))
	{
		iHeight = cView.GetHeight();
	}
	else
	{
		iHeight = mcActualBounds.GetHeight();
	}

	pFont = GetFont();
	if (pFont.IsNotNull() && (iHeight < pFont->GetLineHeight()))
	{
		iHeight = pFont->GetLineHeight();
	}
	return iHeight;
}


//////////////////////////////////////////////////////////////////////////
//
// The row a page of the view below or above uiRow.  The view scrolls as
// far as that row is from uiRow, so the caret moved there stays where it
// was in the view.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::ScrollPage(size uiRow, bool bDown)
{
	STextLayoutLine*	psLine;
	int32				iTop;
	int32				iMiddle;
	int32				iPage;
	size				uiPageRow;

	psLine = mpLayout->GetLayoutLine(uiRow);
	iTop = psLine->iTop;
	iMiddle = psLine->iTop + psLine->iHeight / 2;
	iPage = bDown ? GetPageHeight() : -GetPageHeight();

	uiPageRow = FindRowAt(iMiddle + iPage);
	ScrollBy(mpLayout->GetLayoutLine(uiPageRow)->iTop - iTop);
	return uiPageRow;
}


//////////////////////////////////////////////////////////////////////////
//
// Scrolls the scroll pane the text area is in by iDown pixels.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ScrollBy(int32 iDown)
{
	CInt32Rect	cView;

	if (GetScrollView(&cView))
	{
		cView.SetPos(cView.GetLeft(), cView.GetTop() + iDown);
		ScrollToShow(&cView);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The text goes back to how it was before the last edit kept (Undo), or
// forward to after the next (Redo).  Only the part that differs changes,
// so the spans, backgrounds and gaps of the rest stay, and the caret goes
// after it.  False if there is nothing to undo or redo.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::Undo(void) { return mcEditing.Undo(); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::Redo(void) { return mcEditing.Redo(); }


//////////////////////////////////////////////////////////////////////////
//
// The text goes back, or forward, to the state with id uiId (undoing or
// redoing every edit between).  False if it isn't in the history (any
// more), or the text is in it already.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::UndoTo(size uiId) { return mcEditing.UndoTo(uiId); }


//////////////////////////////////////////////////////////////////////////
//
// Typing that stops for iMilliseconds starts a new edit.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetTypingPause(int32 iMilliseconds)
{
	mcDocument.GetUndo()->SetTypingPause(iMilliseconds);
}


CTextAreaUndoEvent* CTextArea::GetUndoEvent(void) { return &mcUndoEvent; }
CComponentEvent<CTextArea>* CTextArea::GetChangeEvent(void) { return &mcChanged; }  //Called after each edit (undo and redo too), with the text laid out again.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::CanUndo(void)
{
	return mcDocument.CanUndo();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::CanRedo(void)
{
	return mcDocument.CanRedo();
}


//////////////////////////////////////////////////////////////////////////
//
// Which state of the undo history the text is in: the same id again after
// undo or redo comes back to it, so whoever saved the text can tell it is
// as it was saved.  0 for a text area that isn't editable.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::GetUndoId(void)
{
	return mcDocument.GetUndo()->GetCurrentId();
}


//////////////////////////////////////////////////////////////////////////
//
// The undo history is forgotten; an editable text area's starts again
// with the text it has.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ClearUndo(void) { mcEditing.ClearUndo(); }


//////////////////////////////////////////////////////////////////////////
//
// The text is read and laid out again, the caret is kept in view and the
// window lays out again if the text's size changed (an edit isn't made
// while the window lays out, so the layout is invalidated: an edit that
// didn't come from input, which does, still lays the window out again).
// Then the change event is called.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::TextEdited(void)
{
	SInt32Vec2	sTextSize;

	sTextSize = mpLayout->GetTextSize();
	Relayout();
	if (sTextSize != mpLayout->GetTextSize())
	{
		LayoutChanged();
	}
	ShowCaret();
	UpdateCaretLine();

	//An edit shows the caret, even one that doesn't move it.
	if (HasFocus())
	{
		mpWindow->GetFocus()->ResetCaretBlink();
	}
	mcChanged.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The undo event says what was kept, undone or redone.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UndoChanged(STextUndoChange* psChange)
{
	mcUndoEvent.Call(this, psChange);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::MarksChanged(void)
{
	RequestOccurrences();
	UpdateMarks();
	UpdateCaretLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The caret's line highlighted (SetCaretLine) is drawn again on the line
// the caret is on now, if that changed.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UpdateCaretLine(void)
{
	size	uiLine;

	if (!mbCaretLine)
	{
		return;
	}
	uiLine = FindLine(GetCaretPosition());
	if (uiLine != muiCaretLine)
	{
		muiCaretLine = uiLine;
		BackgroundsChanged();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Asks the scroll pane the text area is in to show the caret.  Until the
// text area is laid out it isn't anywhere to show.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ShowCaret(void)
{
	CInt32Rect	cCaret;
	SInt32Vec2	sPosition;

	if ((mcActualBounds.GetWidth() <= 0) || !HasRows())
	{
		return;
	}

	GetCaretBounds(&cCaret);
	sPosition = GetPosition();
	cCaret.Init(cCaret.GetLeft() + sPosition.x, cCaret.GetTop() + sPosition.y, cCaret.GetRight() + sPosition.x, cCaret.GetBottom() + sPosition.y);
	ScrollToShow(&cCaret);
}


//////////////////////////////////////////////////////////////////////////
//
// While it has focus: the caret in the window, cut to what of the text
// area shows.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::GetCaretRect(CInt32Rect* pcDest, ARGB32* psColour)
{
	CInt32Rect	cCaret;
	CInt32Rect	cClip;

	if (!HasFocus() || !HasRows())
	{
		return false;
	}
	GetCaretBounds(&cCaret);
	pcDest->Init(mcActualBounds.GetLeft() + cCaret.GetLeft(), mcActualBounds.GetTop() + cCaret.GetTop(), mcActualBounds.GetLeft() + cCaret.GetRight(), mcActualBounds.GetTop() + cCaret.GetBottom());
	pcDest->ClipTo(&mcActualBounds);
	if (GetClip(&cClip))
	{
		pcDest->ClipTo(&cClip);
	}
	*psColour = msCaretColour;
	return (pcDest->GetWidth() > 0) && (pcDest->GetHeight() > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The selection is drawn as a range (CTextAreaShapes::AddRange).  The caret is
// drawn by the window, blinking, over what it paints (GetCaretRect).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UpdateMarks(void)
{
	size		uiStart;
	size		uiEnd;
	bool		bSelected;
	STextBox*	psBox;

	//The selection is drawn with the text: the rows it changed are drawn again.
	RowsChanged();

	mpSelectionMap->Clear();
	if (!HasRows())
	{
		return;
	}

	//The selected text's other occurrences, once found for it, under the selection.
	bSelected = GetSelection(&uiStart, &uiEnd);
	if (mcOccurrences.IsFoundFor(uiStart, uiEnd))
	{
		mcOccurrences.AddFound(&mcShapes, mpSelectionMap, msOccurrenceColour);
	}

	if (bSelected)
	{
		mcShapes.AddRange(mpSelectionMap, uiStart, uiEnd, msSelectionColour);
		return;
	}

	//A box selection, with no width a line in the caret's colour on each row.
	if (mcEditing.IsBox())
	{
		psBox = mcEditing.GetBox();
		mcShapes.AddBox(mpSelectionMap, psBox->TopRow(), psBox->BottomRow(), psBox->Left(), psBox->Right(), psBox->HasWidth() ? msSelectionColour : msCaretColour);
		return;
	}

	//The ranges given, a symbol's usages say, only while nothing is selected.
	mcOccurrences.AddRanges(&mcShapes, mpSelectionMap, msOccurrenceColour, msWriteOccurrenceColour);
}


//////////////////////////////////////////////////////////////////////////
//
// Ranges of the text drawn as occurrences while nothing is selected, as
// IntelliJ IDEA shows the usages of the symbol at the caret: in the
// occurrence colour, or with bWrite (a declaration) the write occurrence
// colour.  They replace the ones given before, and go when the text
// changes.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetOccurrenceRanges(STextOccurrenceRange* asRanges, size uiNumRanges)
{
	mcOccurrences.SetRanges(asRanges, uiNumRanges);
	UpdateMarks();
}


void CTextArea::ClearOccurrenceRanges(void)
{
	if (mcOccurrences.ClearRanges())
	{
		UpdateMarks();
	}
}


size CTextArea::NumOccurrenceRanges(void) { return mcOccurrences.NumRanges(); }
STextOccurrenceRange* CTextArea::GetOccurrenceRange(size uiIndex) { return mcOccurrences.GetRange(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// The selection or the text may have changed, or highlighting been turned
// on: if the selected text's occurrences aren't the ones found, they are
// found at the next tick (FindOccurrences), once however many keys or
// moves come before it.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::RequestOccurrences(void)
{
	size	uiStart;
	size	uiEnd;

	GetSelection(&uiStart, &uiEnd);
	if (HasRows() && mcOccurrences.Request(uiStart, uiEnd))
	{
		mpWindow->Post(this, &CTextArea::FindOccurrences);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Finds the selected text's other occurrences in the text (see
// CTextAreaOccurrences) and draws them.  Posted at a tick by
// RequestOccurrences.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::FindOccurrences(void)
{
	size	uiStart;
	size	uiEnd;

	GetSelection(&uiStart, &uiEnd);
	if (mcOccurrences.Find(uiStart, uiEnd))
	{
		UpdateMarks();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The selected text's other occurrences in the text are drawn under it in
// the occurrence colour (the theme's TC_TextOccurrence), as IntelliJ
// IDEA's editor shows them, found again at a tick after the selection or
// the text changes.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetHighlightOccurrences(bool bHighlight)
{
	mcOccurrences.SetHighlighting(bHighlight);
	RequestOccurrences();
	UpdateMarks();
}


bool CTextArea::IsHighlightingOccurrences(void) { return mcOccurrences.IsHighlighting(); }
STextRange* CTextArea::GetOccurrence(size uiIndex) { return mcOccurrences.GetFound(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// The occurrences found for the selection as it is; 0 until they are.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::NumOccurrences(void)
{
	size	uiStart;
	size	uiEnd;

	GetSelection(&uiStart, &uiEnd);
	return mcOccurrences.IsFoundFor(uiStart, uiEnd) ? mcOccurrences.NumFound() : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The colours occurrences are drawn in: the selected text's and usages,
// and the usages that are written.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetOccurrenceColours(ARGB32 sOccurrence, ARGB32 sWrite)
{
	msOccurrenceColour = sOccurrence;
	msWriteOccurrenceColour = sWrite;
	UpdateMarks();
}


//////////////////////////////////////////////////////////////////////////
//
// Something drawn on the rows may have changed: the window is painted, and
// what changed found as it is (PrepareArea).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::RowsChanged(void)
{
	mbRowsChanged = true;
	if (mpWindow.IsNotNull())
	{
		mpWindow->RequestPaint();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The backgrounds' map is drawn again before it is next drawn.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::BackgroundsChanged(void)
{
	mbBackgroundsDirty = true;
	RowsChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Draws the backgrounds into their map, each over the ones before it,
// across the text area or the text if that is wider.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UpdateBackgrounds(void)
{
	mpBackgroundMap->Clear();
	miBackgroundWidth = GetFullWidth();
	mbBackgroundsDirty = false;
	if (mbCaretLine && (muiCaretLine < NumLines()))
	{
		mcShapes.AddLine(mpBackgroundMap, muiCaretLine, msCaretLineColour, miBackgroundWidth);
	}
	mcShapes.AddBackgrounds(mpBackgroundMap, &mcBackgrounds, miBackgroundWidth);
}


//////////////////////////////////////////////////////////////////////////
//
// The width of the laid out text, or of the text area if that is wider.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextArea::GetFullWidth(void)
{
	int32	iTextWidth;

	iTextWidth = mpLayout->GetTextSize().x;
	return (mcActualBounds.GetWidth() > iTextWidth) ? mcActualBounds.GetWidth() : iTextWidth;
}


bool CTextArea::IsEditable(void) { return mcEditing.IsEditable(); }
bool CTextArea::IsSelectable(void) { return mbSelectable; }
CTextEditing* CTextArea::GetEditing(void) { return &mcEditing; }  //The caret, the selection and the edits: its moves and edits are made through it.
size CTextArea::GetCaretPosition(void) { return mcEditing.GetCaretPosition(); }


//////////////////////////////////////////////////////////////////////////
//
// Draws a fixed width font as tiles, each character in a cell of a tile
// layer, instead of as sprites.  Only used without word wrap.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetTiledText(bool bTiledText)
{
	if (mpLayout->SetTiledText(bTiledText))
	{
		Relayout();

		//What it is laid out from has changed.
		InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::IsTiledText(void) { return mpLayout->IsTiledText(); }


//////////////////////////////////////////////////////////////////////////
//
// Tiles were asked for and the font and word wrap allow them.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::IsDrawnAsTiles(void) { return mpLayout->IsTiled(); }  //Tiles were asked for and the font and word wrap allow them.


//////////////////////////////////////////////////////////////////////////
//
// The editing's view: the rows the layout has the text in.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::HasRows(void) { return mpLayout->HasRows(); }
size CTextArea::NumRows(void) { return mpLayout->NumLayoutLines(); }
size CTextArea::FindRow(size uiPosition, bool bRowEnd) { return mpLayout->FindRow(uiPosition, bRowEnd); }
size CTextArea::GetRowStart(size uiRow) { return mpLayout->GetRowStart(uiRow); }
size CTextArea::GetRowEnd(size uiRow, bool* pbRowEnd) { return mpLayout->GetRowEnd(uiRow, pbRowEnd); }
size CTextArea::FindRowAt(int32 y) { return mpLayout->FindRowAt(y); }
size CTextArea::FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd) { return mpLayout->FindPositionInRow(uiRow, x, pbRowEnd); }
int32 CTextArea::GetPositionX(size uiPosition, bool bRowEnd) { return mpLayout->GetPositionX(uiPosition, bRowEnd); }


//////////////////////////////////////////////////////////////////////////
//
// The text is read and laid out again (as when what it is laid out from
// changed), then the marks drawn over it again.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Relayout(void)
{
	mpLayout->ReadText();
	mpLayout->LayoutText();
	LaidOut();
}


//////////////////////////////////////////////////////////////////////////
//
// Spans, a style or gaps set since the text was last laid out are laid
// out (see CTextAreaLayout::UpdateText).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UpdateText(void)
{
	if (mpLayout->UpdateText())
	{
		LaidOut();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Before the size is asked for, spans or a style that can change it are
// applied.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UpdateSize(void)
{
	if (mpLayout->UpdateSize())
	{
		LaidOut();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The text was laid out again: the backgrounds and selection are drawn
// on its rows again.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::LaidOut(void)
{
	mbBackgroundsDirty = true;
	RequestOccurrences();
	UpdateMarks();
}



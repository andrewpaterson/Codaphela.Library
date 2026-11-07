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
#include "TextArea.h"


//The pointer's commands: presses, releases and dragging to select.
static EComponentInputCommand	gaeTextAreaPointerCommands[] = { CIC_ActivateComponent, CIC_ExecuteComponent, CIC_StartDragSelect, CIC_ContDragSelect, CIC_StopDragSelect };


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
	mpFamily = pFamily;
	msStyle.Init();
	mcSpans.Init();
	mbTextDirty = false;
	mbSizeDirty = false;
	mpText = NULL;
	mbTextStale = false;
	mpLayout = OMalloc<CTextLayout>();
	mpSpriteMap = OMalloc<CSpriteMap>();
	mpGeometryMap = OMalloc<CGeometryMap>();
	mpFontDraw = OMalloc<CMapFontDraw>(mpSpriteMap, mpGeometryMap);
	mpBackgroundMap = OMalloc<CGeometryMap>();
	mcBackgrounds.Init();
	mbBackgroundsDirty = false;
	miBackgroundWidth = 0;
	mpSelectionMap = OMalloc<CGeometryMap>();
	mbMarksDirty = false;
	mpMaps = OMalloc<CMaps>();
	mbTiledText = false;
	mbTiled = false;
	mpTiles = OMalloc<CTextAreaTiles>(&mcDocument, &mauiStopStarts, &mcGaps, &msStyle);
	mpMaps->AddMap(mpBackgroundMap);
	mpMaps->AddMap(mpSelectionMap);
	mpMaps->AddMap(mpSpriteMap);
	mpMaps->AddMap(mpTiles->GetMap());
	mpMaps->AddMap(mpGeometryMap);
	mpMaps->SetViewportPosition(0, 0);
	mbWordWrap = false;
	msTextSize.Init(0, 0);
	msSelectionColour = pTheme->GetColour(TC_TextSelection);
	msCaretColour = pTheme->GetColour(TC_Caret);
	mauiStopStarts.Init();
	mcGaps.Init();
	mcDocument.AddListener(&mcSpans);
	mcDocument.AddListener(&mcBackgrounds);
	mcDocument.AddListener(&mcGaps);
	mcEditing.Init(&mcDocument, this);
	mcUndoEvent.Init();
	mcChanged.Init();
	mbSelectable = true;
	mbDragging = false;

	//The text is drawn over whatever is behind the text area.
	pDraw = OMalloc<CMapsCanvasDraw>(mpMaps);
	CCanvas::Init(pWindow, pDraw);
	SetTransparent(true);
	SetCanGetFocus(true);

	ReadText();
	LayoutText();

	AddInputAction(CIC_ActivateComponent, this, &CTextArea::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CTextArea::ExecuteComponent);
	AddInputAction(CIC_ContextMenu, this, &CTextArea::ContextMenuClick);
	AddInputAction(CIC_StartDragSelect, this, &CTextArea::StartDragSelect);
	AddInputAction(CIC_ContDragSelect, this, &CTextArea::ContDragSelect);
	AddInputAction(CIC_StopDragSelect, this, &CTextArea::StopDragSelect);

	AddInputAction(CIC_Up, this, &CTextArea::Up);
	AddInputAction(CIC_Down, this, &CTextArea::Down);
	AddInputAction(CIC_Left, this, &CTextArea::Left);
	AddInputAction(CIC_Right, this, &CTextArea::Right);
	AddInputAction(CIC_WordLeft, this, &CTextArea::WordLeft);
	AddInputAction(CIC_WordRight, this, &CTextArea::WordRight);
	AddInputAction(CIC_Home, this, &CTextArea::Home);
	AddInputAction(CIC_End, this, &CTextArea::End);
	AddInputAction(CIC_DocumentHome, this, &CTextArea::DocumentHome);
	AddInputAction(CIC_DocumentEnd, this, &CTextArea::DocumentEnd);
	AddInputAction(CIC_PageUp, this, &CTextArea::PageUp);
	AddInputAction(CIC_PageDown, this, &CTextArea::PageDown);
	AddInputAction(CIC_LineUp, this, &CTextArea::LineUp);
	AddInputAction(CIC_LineDown, this, &CTextArea::LineDown);

	AddInputAction(CIC_SelectUp, this, &CTextArea::SelectUp);
	AddInputAction(CIC_SelectDown, this, &CTextArea::SelectDown);
	AddInputAction(CIC_SelectLeft, this, &CTextArea::SelectLeft);
	AddInputAction(CIC_SelectRight, this, &CTextArea::SelectRight);
	AddInputAction(CIC_SelectWordLeft, this, &CTextArea::SelectWordLeft);
	AddInputAction(CIC_SelectWordRight, this, &CTextArea::SelectWordRight);
	AddInputAction(CIC_SelectHome, this, &CTextArea::SelectHome);
	AddInputAction(CIC_SelectEnd, this, &CTextArea::SelectEnd);
	AddInputAction(CIC_SelectDocumentHome, this, &CTextArea::SelectDocumentHome);
	AddInputAction(CIC_SelectDocumentEnd, this, &CTextArea::SelectDocumentEnd);
	AddInputAction(CIC_SelectPageUp, this, &CTextArea::SelectPageUp);
	AddInputAction(CIC_SelectPageDown, this, &CTextArea::SelectPageDown);
	AddInputAction(CIC_SelectWholeWord, this, &CTextArea::SelectWholeWord);
	AddInputAction(CIC_SelectAll, this, &CTextArea::SelectAll);
	AddInputAction(CIC_ClearSelection, this, &CTextArea::ClearSelection);

	AddInputAction(CIC_Space, this, &CTextArea::SpaceAction);
	AddInputAction(CIC_Tab, this, &CTextArea::TabAction);
	AddInputAction(CIC_BackTab, this, &CTextArea::BackTab);
	AddInputAction(CIC_Enter, this, &CTextArea::Enter);
	AddInputAction(CIC_HomeEnter, this, &CTextArea::HomeEnter);
	AddInputAction(CIC_EndEnter, this, &CTextArea::EndEnter);
	AddInputAction(CIC_Delete, this, &CTextArea::Delete);
	AddInputAction(CIC_Backspace, this, &CTextArea::Backspace);
	AddInputAction(CIC_Undo, this, &CTextArea::UndoAction);
	AddInputAction(CIC_Redo, this, &CTextArea::RedoAction);
	AddInputAction(CIC_Duplicate, this, &CTextArea::Duplicate);
	AddInputAction(CIC_DeleteWordRemainingLeft, this, &CTextArea::DeleteWordRemainingLeft);
	AddInputAction(CIC_DeleteWordRemainingRight, this, &CTextArea::DeleteWordRemainingRight);
	AddInputAction(CIC_DeleteLine, this, &CTextArea::DeleteLine);

	AddInputAction(CIC_Cut, this, &CTextArea::CutToClipboard);
	AddInputAction(CIC_Copy, this, &CTextArea::CopyToClipboard);
	AddInputAction(CIC_Paste, this, &CTextArea::PasteFromClipboard);
	AddInputAction(IAF_Printable, NULL, this, &CTextArea::PrintableAction);

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
	mcGaps.Kill();
	mauiStopStarts.Kill();
	mcBackgrounds.Kill();
	mcSpans.Kill();
	mcChanged.Kill();
	mcUndoEvent.Kill();
	mcEditing.Kill();
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
	M_Pointer(mpFamily);
	U_Data(STextStyle, msStyle);
	U_Unknown(CTextSpans, mcSpans);
	U_Bool(mbTextDirty);
	U_Bool(mbSizeDirty);
	U_Unknown(CTextAreaUndoEvent, mcUndoEvent);
	U_Unknown(CComponentEvent<CTextArea>, mcChanged);
	M_Pointer(mpText);
	U_Bool(mbTextStale);
	M_Pointer(mpLayout);
	M_Pointer(mpAppendLayout);
	M_Pointer(mpSpriteMap);
	M_Pointer(mpGeometryMap);
	M_Pointer(mpFontDraw);
	M_Pointer(mpMaps);
	U_Bool(mbWordWrap);
	U_2Int32(msTextSize);
	U_Bool(mbTiledText);
	U_Bool(mbTiled);
	M_Pointer(mpTiles);
	M_Pointer(mpBackgroundMap);
	U_Unknown(CTextBackgrounds, mcBackgrounds);
	U_Bool(mbBackgroundsDirty);
	U_Int32(miBackgroundWidth);
	M_Pointer(mpSelectionMap);
	U_Bool(mbMarksDirty);
	U_UInt32(msSelectionColour);
	U_UInt32(msCaretColour);
	U_Unknown(CArrayTextAreaStopStart, mauiStopStarts);
	U_Unknown(CTextLineGaps, mcGaps);
	U_Unknown(CTextEditing, mcEditing);
	U_Bool(mbSelectable);
	U_Bool(mbDragging);
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
	mcSpans.Clear();
	mcBackgrounds.Clear();
	mbBackgroundsDirty = true;
	mcGaps.Clear();
	mcEditing.Reset();
	ReadText();
	LayoutText();
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
	size	uiStart;
	bool	bAppendTiles;
	bool	bAppendSprites;

	if ((szText == NULL) || (szText[0] == '\0'))
	{
		return;
	}
	bAppendTiles = CanAppendTiles();
	bAppendSprites = !bAppendTiles && CanAppendSprites();
	uiStart = mcDocument.Length();
	mcDocument.Append(szText);
	AddSpan(uiStart, mcDocument.Length() - uiStart, sColour);
	if (bAppendTiles)
	{
		AppendTiles(uiStart);
		mbTextDirty = false;
		LayoutTiles();
	}
	else if (bAppendSprites)
	{
		AppendSprites(uiStart);
		mbTextDirty = false;
	}
	else
	{
		ReadText();
		LayoutText();
	}

	//What it is laid out from has changed.
	InvalidateLayout();
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
	mpFamily = pFamily;
	ReadText();
	LayoutText();

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetWordWrap(bool bWordWrap)
{
	if (mbWordWrap != bWordWrap)
	{
		mbWordWrap = bWordWrap;
		ReadText();
		LayoutText();
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
	if (!msStyle.EqualsFont(psStyle))
	{
		mbSizeDirty = true;
	}
	msStyle = *psStyle;
	mbTextDirty = true;

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::GetStyle(STextStyle* psStyle)
{
	*psStyle = msStyle;
}


//////////////////////////////////////////////////////////////////////////
//
// The colour of text that isn't in a span that sets a colour.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetColour(ARGB32 sColour)
{
	if (msStyle.sColour != sColour)
	{
		msStyle.sColour = sColour;
		mbTextDirty = true;
	}
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
	size	uiTextLength;

	uiTextLength = mcDocument.Length();
	if ((uiStart > uiTextLength) || (uiLength > uiTextLength - uiStart))
	{
		return false;
	}
	if ((uiLength == 0) || (psOverride->uiSet == 0))
	{
		return true;
	}

	mcSpans.Add(uiStart, uiLength, psOverride);
	mbTextDirty = true;
	if (psOverride->ChangesFont())
	{
		mbSizeDirty = true;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ClearSpans(void)
{
	if (mcSpans.ChangesFont())
	{
		mbSizeDirty = true;
	}
	if (mcSpans.NumSpans() > 0)
	{
		mcSpans.Clear();
		mbTextDirty = true;
	}
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
	mbBackgroundsDirty = true;
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
	mbBackgroundsDirty = true;
	return true;
}


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
	mbBackgroundsDirty = true;
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
		mbBackgroundsDirty = true;
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
// The style of the byte at uiPosition of the UTF8 text: the text area's
// style with every span that covers it applied in the order they were
// added.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::GetStyleAt(size uiPosition, STextStyle* psStyle)
{
	*psStyle = msStyle;
	mcSpans.ApplyAt(uiPosition, psStyle);
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

	if (mbTextDirty || (!mbTiled && (mpLayout->GetWidth() != GetWrapWidth())))
	{
		//Wrapped to a new width, or in new styles, the text can be a different size so the window must lay its
		//components out again.
		sTextSize = msTextSize;
		if (mbTextDirty)
		{
			ReadText();
		}
		LayoutText();
		if ((msTextSize.x != sTextSize.x) || (msTextSize.y != sTextSize.y))
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
	mbMarksDirty = false;
	return CCanvas::Draw();
}


//////////////////////////////////////////////////////////////////////////
//
// The text area's setters only note that its text, spans, backgrounds or
// selection changed (Draw brings them up to date), so it draws again here
// when any did.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::PrepareArea(bool bDrawn, bool bClipped, CInt32Rect* pcClip, CInt32Rect* pcArea)
{
	CCanvas::PrepareArea(bDrawn, bClipped, pcClip, pcArea);
	if (mbTextDirty || mbBackgroundsDirty || mbMarksDirty || (miBackgroundWidth != GetFullWidth()))
	{
		MarkContentChanged();
		mpWindow->AddDamage(pcArea);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UpdateText(void)
{
	if (mbTextDirty)
	{
		ReadText();
		LayoutText();
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
	if (mbSizeDirty)
	{
		UpdateText();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The width lines are wrapped to, -1 if they aren't.  A text area that
// hasn't been laid out yet has no width so its lines aren't wrapped.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextArea::GetWrapWidth(void)
{
	int32	iWidth;

	iWidth = mcActualBounds.GetWidth();
	if (mbWordWrap && (iWidth > 0))
	{
		return iWidth;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the text from the UTF8 in the fonts of its styles.  Without a font
// for the text area's style there is no text (the family logs why).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ReadText(void)
{
	mbTextDirty = false;
	mbSizeDirty = false;
	mbTextStale = false;
	FindStopStarts(0);

	mbTiled = UseTiles();
	if (mbTiled)
	{
		ReadTiles();
		return;
	}
	mpTiles->ClearTiles();

	mpText = ReadTextFrom(0);
}


//////////////////////////////////////////////////////////////////////////
//
// The style of each byte of the text from uiFrom on (the first is
// uiFrom's): the text area's style with the spans over it, applied in the
// order they were added.  NULL if there are no spans, when every byte is in
// the text area's style.  Free it.
//
//////////////////////////////////////////////////////////////////////////
STextStyle* CTextArea::MakeStyles(size uiFrom)
{
	size		uiTextLength;
	size		uiPosition;
	STextStyle*	psStyles;

	uiTextLength = mcDocument.Length();
	if ((mcSpans.NumSpans() == 0) || (uiTextLength <= uiFrom))
	{
		return NULL;
	}

	psStyles = (STextStyle*)malloc((uiTextLength - uiFrom) * sizeof(STextStyle));
	for (uiPosition = uiFrom; uiPosition < uiTextLength; uiPosition++)
	{
		psStyles[uiPosition - uiFrom] = msStyle;
	}
	mcSpans.Apply(psStyles, uiFrom, uiTextLength);
	return psStyles;
}


//////////////////////////////////////////////////////////////////////////
//
// The text area's style, or a span, underlines, overlines or strikes
// through some of the text.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::HasDecorations(void)
{
	if (msStyle.GetDecorations() != 0)
	{
		return true;
	}
	return mcSpans.HasDecorations(&msStyle);
}


//////////////////////////////////////////////////////////////////////////
//
// A font change is made wherever the style of the text from uiFrom on
// (MakeStyles) changes, at its byte after uiFrom.  Text before the first
// change is in pDefaultFont.  A style the family has no font for is drawn
// in pDefaultFont (the family logs an error).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::MakeFontChanges(CArrayTextFontChange* pasChanges, Ptr<CFont> pDefaultFont, int16 iDefaultBaselineShift, size uiFrom)
{
	size				uiTextLength;
	size				uiPosition;
	STextStyle*			psStyles;
	STextStyle*			psCurrent;
	Ptr<CFont>			pFont;
	int16				iBaselineShift;

	psStyles = MakeStyles(uiFrom);
	if (psStyles == NULL)
	{
		return;
	}

	uiTextLength = mcDocument.Length() - uiFrom;
	psCurrent = &msStyle;
	for (uiPosition = 0; uiPosition < uiTextLength; uiPosition++)
	{
		if (!psStyles[uiPosition].Equals(psCurrent))
		{
			pFont = mpFamily->GetRunFont(&psStyles[uiPosition], &iBaselineShift);
			if (pFont.IsNull())
			{
				pFont = pDefaultFont;
				iBaselineShift = iDefaultBaselineShift;
			}
			pasChanges->Add()->Init(uiPosition, &pFont, iBaselineShift, psStyles[uiPosition].GetDecorations(), psStyles[uiPosition].sColour);
			psCurrent = &psStyles[uiPosition];
		}
	}
	SafeFree(psStyles);
}


//////////////////////////////////////////////////////////////////////////
//
// Places a sprite for each glyph, wrapped to the actual width if word wrap
// is on, and works out the size they need.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::LayoutText(void)
{
	Ptr<CFont>			pFont;
	STextLayoutLine*	psLine;

	if (mbTextStale)
	{
		//Text was appended since the whole was read: it is all laid out again, so it is all read again.
		ReadText();
	}
	if (mbTiled)
	{
		LayoutTiles();
		return;
	}

	mpLayout->SetWidth(GetWrapWidth());
	mpLayout->SetLineGaps(mcGaps.GetGaps());
	mpLayout->SetTabWidth(GetTabPixels());

	mpFontDraw->Begin();
	if (mpText.IsNotNull())
	{
		mpLayout->Layout(mpFontDraw, mpText);
		msTextSize = mpLayout->GetSize();
	}
	else
	{
		msTextSize.Init(0, 0);
	}
	mpFontDraw->End();

	//Even a text area with no text is a line high, above its gap.  With no characters the layout had no font to make
	//its line high.
	pFont = GetFont();
	if (pFont.IsNotNull() && (mcDocument.Length() == 0) && mpText.IsNotNull() && (mpLayout->NumLines() == 1))
	{
		psLine = mpLayout->GetLine(0);
		psLine->iHeight = pFont->GetLineHeight();
		psLine->iBaseline = pFont->GetAscent();
		msTextSize.y = psLine->iHeight + GetLineGap(0);
	}
	if (pFont.IsNotNull() && (msTextSize.y < pFont->GetLineHeight()))
	{
		msTextSize.y = pFont->GetLineHeight();
	}

	mbBackgroundsDirty = true;
	UpdateMarks();
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
	sSize = msTextSize;
	if (mbWordWrap)
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
	return msTextSize;
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels between tab stops when the text is laid out as sprites: the tab
// width's spaces of the font.  0 (tabs have no width) without a font or a
// space in it.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextArea::GetTabPixels(void)
{
	Ptr<CFont>		pFont;
	Ptr<CGlyph>		pSpace;

	pFont = GetFont();
	if (pFont.IsNull())
	{
		return 0;
	}
	pSpace = pFont->GetGlyph((uint16)' ', 1);
	if (pSpace.IsNull())
	{
		return 0;
	}
	return (int32)mcEditing.GetTabWidth() * pSpace->GetFullWidth();
}


//////////////////////////////////////////////////////////////////////////
//
// The colours of the text area's style's face and size.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFontColours> CTextArea::GetFontColours(void)
{
	return mpFamily->GetColours(msStyle.GetFontStyle(), msStyle.fPixelSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The white font of the text area's style's face and size.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CTextArea::GetFont(void)
{
	Ptr<CFontColours>	pColours;

	pColours = GetFontColours();
	if (pColours.IsNull())
	{
		return NULL;
	}
	return pColours->GetWhite();
}


char* CTextArea::GetText(void) { return mcDocument.Text(); }
size CTextArea::GetChangeCount(void) { return mcDocument.GetChangeCount(); }  //Each edit counts one more, so the text's owner can tell it changed since it last looked.
Ptr<CFontFamily> CTextArea::GetFontFamily(void) { return mpFamily; }
bool CTextArea::IsWordWrap(void) { return mbWordWrap; }
ARGB32 CTextArea::GetColour(void) { return msStyle.sColour; }
size CTextArea::NumSpans(void) { return mcSpans.NumSpans(); }


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
	size	i;

	mbSelectable = bSelectable;
	for (i = 0; i < sizeof(gaeTextAreaPointerCommands) / sizeof(EComponentInputCommand); i++)
	{
		SetInputActionsEnabled(gaeTextAreaPointerCommands[i], bSelectable);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many spaces apart the tab stops are.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetTabWidth(int32 iTabWidth)
{
	mcEditing.SetTabWidth(iTabWidth < 1 ? 1 : (size)iTabWidth);

	//Tabs go to tab stops, as tiles or as sprites, so the text is laid out again.
	if (mbTiled)
	{
		ReadText();
	}
	LayoutText();

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
// uiPosition is a byte of the text and should be the start of a
// character.  Nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetCaretPosition(size uiPosition)
{
	mcEditing.SetCaretPosition(uiPosition);

	//What it is laid out from has changed.
	InvalidateLayout();
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

	//What it is laid out from has changed.
	InvalidateLayout();
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
// The caret's pixels relative to the text area's top left, as it would be
// drawn.  It is a pixel wide and as high as its row.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::GetCaretBounds(CInt32Rect* pcDest)
{
	STextLayoutLine*	psLine;
	int32				x;

	UpdateText();
	if (!HasRows())
	{
		pcDest->Init(0, 0, 1, msTextSize.y);
		return;
	}

	psLine = GetLayoutLine(mcEditing.GetCaretRow());
	x = mcEditing.GetCaretX();
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

	psFirst = GetLayoutLine(GetLayoutStop(FindStop(GetLineStart(uiLine)))->uiLine);
	psLast = GetLayoutLine(FindLastRow(uiLine));
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
	if (mcGaps.Set(uiLine, iHeight))
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
	return mcGaps.GetId(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
// The line the gap with id uiId is below, MAX_SIZE if it has gone.  It
// looks through every gap; to find many, go through GetLineGaps once.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindLineGap(uint32 uiId)
{
	return mcGaps.FindLine(uiId);
}


//////////////////////////////////////////////////////////////////////////
//
// The gaps, sorted by line.  Don't change them.
//
//////////////////////////////////////////////////////////////////////////
CArrayTextLayoutGap* CTextArea::GetLineGaps(void)
{
	return mcGaps.GetGaps();
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
	if (mcGaps.Copy(pasGaps, NumLines()))
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
	return mcGaps.GetHeight(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::NumLineGaps(void)
{
	return mcGaps.NumGaps();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ClearLineGaps(void)
{
	if (mcGaps.Clear())
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

	uiRow = FindLastRow(uiLine);
	psLine = GetLayoutLine(uiRow);
	iTop = psLine->iTop + psLine->iHeight;
	iBottom = (uiRow + 1 < NumLayoutLines()) ? GetLayoutLine(uiRow + 1)->iTop : msTextSize.y;
	iWidth = (mcActualBounds.GetWidth() > msTextSize.x) ? mcActualBounds.GetWidth() : msTextSize.x;
	pcDest->Init(0, iTop, iWidth, iBottom);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The moves and edits of the text area's editing (see CTextEditing).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Up(void) { mcEditing.Up(); }
void CTextArea::Down(void) { mcEditing.Down(); }
void CTextArea::Left(void) { mcEditing.Left(); }
void CTextArea::Right(void) { mcEditing.Right(); }
void CTextArea::WordLeft(void) { mcEditing.WordLeft(); }
void CTextArea::WordRight(void) { mcEditing.WordRight(); }
void CTextArea::Home(void) { mcEditing.Home(); }
void CTextArea::End(void) { mcEditing.End(); }
void CTextArea::DocumentHome(void) { mcEditing.DocumentHome(); }
void CTextArea::DocumentEnd(void) { mcEditing.DocumentEnd(); }
void CTextArea::PageUp(void) { mcEditing.PageUp(); }
void CTextArea::PageDown(void) { mcEditing.PageDown(); }
void CTextArea::SelectUp(void) { mcEditing.SelectUp(); }
void CTextArea::SelectDown(void) { mcEditing.SelectDown(); }
void CTextArea::SelectLeft(void) { mcEditing.SelectLeft(); }
void CTextArea::SelectRight(void) { mcEditing.SelectRight(); }
void CTextArea::SelectWordLeft(void) { mcEditing.SelectWordLeft(); }
void CTextArea::SelectWordRight(void) { mcEditing.SelectWordRight(); }
void CTextArea::SelectHome(void) { mcEditing.SelectHome(); }
void CTextArea::SelectEnd(void) { mcEditing.SelectEnd(); }
void CTextArea::SelectDocumentHome(void) { mcEditing.SelectDocumentHome(); }
void CTextArea::SelectDocumentEnd(void) { mcEditing.SelectDocumentEnd(); }
void CTextArea::SelectPageUp(void) { mcEditing.SelectPageUp(); }
void CTextArea::SelectPageDown(void) { mcEditing.SelectPageDown(); }
void CTextArea::SelectAll(void) { Select(0, mcDocument.Length()); }
void CTextArea::ClearSelection(void) { mcEditing.ClearSelection(); }
void CTextArea::Printable(char c, bool bInsert) { mcEditing.Printable(c, bInsert); }
void CTextArea::Tab(bool bInsert) { mcEditing.Tab(bInsert); }
void CTextArea::BackTab(void) { mcEditing.BackTab(); }
void CTextArea::Enter(void) { mcEditing.Enter(); }
void CTextArea::HomeEnter(void) { mcEditing.HomeEnter(); }
void CTextArea::EndEnter(void) { mcEditing.EndEnter(); }
void CTextArea::Delete(void) { mcEditing.Delete(); }
void CTextArea::Backspace(void) { mcEditing.Backspace(); }
void CTextArea::Copy(CChars* pszDest) { mcEditing.Copy(pszDest); }
void CTextArea::Cut(CChars* pszDest) { mcEditing.Cut(pszDest); }
void CTextArea::CutLine(CChars* pszDest) { mcEditing.CutLine(pszDest); }
void CTextArea::Paste(char* szSource) { mcEditing.Paste(szSource); }
void CTextArea::Duplicate(void) { mcEditing.Duplicate(); }
void CTextArea::DeleteWordRemainingRight(void) { mcEditing.DeleteWordRemainingRight(); }
void CTextArea::DeleteWordRemainingLeft(void) { mcEditing.DeleteWordRemainingLeft(); }
void CTextArea::DeleteLine(void) { mcEditing.DeleteLine(); }
bool CTextArea::ReplaceRanges(size* auiStarts, size* auiEnds, size uiNumRanges, const char* szText) { return mcEditing.ReplaceRanges(auiStarts, auiEnds, uiNumRanges, szText); }


//////////////////////////////////////////////////////////////////////////
//
// Selects the word the caret is in or next to.  Not next to a word,
// nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SelectWholeWord(void)
{
	mcEditing.SelectWholeWord();

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// A space is typed as a printable character; tab never overwrites.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SpaceAction(SComponentInputEvent* psEvent) { Printable(' ', psEvent->bInsert); }
void CTextArea::TabAction(SComponentInputEvent* psEvent) { Tab(psEvent->bInsert); }


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


//////////////////////////////////////////////////////////////////////////
//
// Undo and redo only with something to undo or redo; cut and paste only
// in editable text.
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
	mbDragging = false;
	UpdateMarks();
}


//////////////////////////////////////////////////////////////////////////
//
// The clipboard is the window's, the system's.  Cutting or copying
// nothing leaves it as it was.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::CutToClipboard(void) { mpWindow->GetClipboard()->CopyFrom(this, &CTextArea::Cut); }
void CTextArea::CopyToClipboard(void) { mpWindow->GetClipboard()->CopyFrom(this, &CTextArea::Copy); }
void CTextArea::PasteFromClipboard(void) { mpWindow->GetClipboard()->PasteInto(this, &CTextArea::Paste); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::PrintableAction(SComponentInputEvent* psEvent)
{
	Printable(psEvent->cPrintable, psEvent->bInsert);
}


//////////////////////////////////////////////////////////////////////////
//
// The press has already given the text area focus.  The caret goes to the
// gap nearest the pointer and dragging selects from there.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ActivateComponent(int x, int y)
{
	size	uiPosition;
	bool	bRowEnd;

	bRowEnd = false;
	uiPosition = 0;
	if (HasRows())
	{
		uiPosition = FindPositionInRow(FindRowAt(y), x, &bRowEnd);
	}
	mcEditing.MoveCaret(uiPosition, false, bRowEnd);
	mpWindow->GetFocus()->StartDrag(FD_Select, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// The right button, as in Visual Studio: pressed in the selection it is
// kept, so the context menu acts on it; anywhere else the caret moves
// there and nothing is selected.  The press goes on up to the component
// with the context menu.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ContextMenuClick(SComponentInputEvent* psEvent)
{
	size	uiPosition;
	size	uiStart;
	size	uiEnd;
	bool	bRowEnd;

	psEvent->bPass = true;

	bRowEnd = false;
	uiPosition = 0;
	if (HasRows())
	{
		uiPosition = FindPositionInRow(FindRowAt(psEvent->y), psEvent->x, &bRowEnd);
	}
	if (GetSelection(&uiStart, &uiEnd) && (uiPosition >= uiStart) && (uiPosition < uiEnd))
	{
		return;
	}
	mcEditing.MoveCaret(uiPosition, false, bRowEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ExecuteComponent(int x, int y)
{
	mpWindow->GetFocus()->StopDrag(FD_Select);
}


//////////////////////////////////////////////////////////////////////////
//
// A shift press extends the selection from where it started, or from the
// caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::StartDragSelect(int x, int y)
{
	mcEditing.AnchorSelection();
	mbDragging = true;
	ContDragSelect(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ContDragSelect(int x, int y)
{
	size	uiPosition;
	bool	bRowEnd;

	if (mbDragging && HasRows())
	{
		uiPosition = FindPositionInRow(FindRowAt(y), x, &bRowEnd);
		mcEditing.MoveCaret(uiPosition, true, bRowEnd);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::StopDragSelect(int x, int y)
{
	ContDragSelect(x, y);
	mbDragging = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The byte each of the layout's stops is before, found the way the text
// is read from uiFrom on (0 for all of it; else the old end, when text
// was appended): a stop for each character but carriage returns, then one
// at the end of the text.  Stops found before at or after uiFrom go.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::FindStopStarts(size uiFrom)
{
	size	uiPosition;
	size	uiLength;
	char*	szText;

	while ((mauiStopStarts.NumElements() > 0) && (*mauiStopStarts.Tail() >= uiFrom))
	{
		mauiStopStarts.RemoveTail();
	}
	szText = mcDocument.Text();
	uiLength = mcDocument.Length();
	uiPosition = uiFrom;
	while (uiPosition < uiLength)
	{
		if (szText[uiPosition] != '\r')
		{
			*mauiStopStarts.Add() = uiPosition;
		}
		uiPosition += mcDocument.GetNavigation()->CharacterLength(uiPosition);
	}
	*mauiStopStarts.Add() = uiLength;
}


//////////////////////////////////////////////////////////////////////////
//
// The row line uiLine ends on: the row of its line feed, or the last row.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindLastRow(size uiLine)
{
	if (uiLine + 1 < NumLines())
	{
		return GetLayoutStop(FindStop(GetLineStart(uiLine + 1) - 1))->uiLine;
	}
	return NumLayoutLines() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Like spans, the gaps are laid out when the text area is next laid out,
// drawn, asked its size or asked where something is, so changing many is
// laid out once.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::GapsChanged(void)
{
	mbTextDirty = true;
	mbSizeDirty = true;

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the layout is of the text as it is.  Without a font there is no
// layout, and the caret and selection aren't drawn.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::HasRows(void)
{
	return (mbTiled || mpText.IsNotNull()) && (NumLayoutLines() > 0) && (NumLayoutStops() == mauiStopStarts.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
// The first stop at or after uiPosition.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindStop(size uiPosition)
{
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;

	uiLow = 0;
	uiHigh = mauiStopStarts.NumElements() - 1;
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		if (*mauiStopStarts.Get(uiMiddle) < uiPosition)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	return uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
// A row ends where a word didn't fit on it, not at a line feed or the end
// of the text.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::IsRowWrapped(size uiRow)
{
	STextLayoutLine*	psLine;
	size				uiLast;

	if (uiRow + 1 >= NumLayoutLines())
	{
		return false;
	}
	psLine = GetLayoutLine(uiRow);
	uiLast = psLine->uiFirstStop + psLine->uiNumStops - 1;
	return mcDocument.Text()[*mauiStopStarts.Get(uiLast)] != '\n';
}


//////////////////////////////////////////////////////////////////////////
//
// The row uiPosition is on.  At the start of a wrapped row and bRowEnd it
// is the end of the row above.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindRow(size uiPosition, bool bRowEnd)
{
	size	uiRow;

	uiRow = GetLayoutStop(FindStop(uiPosition))->uiLine;
	if (bRowEnd && (uiRow > 0) && (GetRowStart(uiRow) == uiPosition) && IsRowWrapped(uiRow - 1))
	{
		uiRow--;
	}
	return uiRow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::GetRowStart(size uiRow)
{
	return *mauiStopStarts.Get(GetLayoutLine(uiRow)->uiFirstStop);
}


//////////////////////////////////////////////////////////////////////////
//
// The last position on the row: before its line feed, at the end of the
// text, or for a wrapped row the start of the next row with pbRowEnd true.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::GetRowEnd(size uiRow, bool* pbRowEnd)
{
	STextLayoutLine*	psLine;

	*pbRowEnd = false;
	if (uiRow + 1 >= NumLayoutLines())
	{
		return mcDocument.Length();
	}

	psLine = GetLayoutLine(uiRow);
	if (IsRowWrapped(uiRow))
	{
		*pbRowEnd = true;
		return *mauiStopStarts.Get(psLine->uiFirstStop + psLine->uiNumStops);
	}
	return mcDocument.GetNavigation()->FindLineEnd(*mauiStopStarts.Get(psLine->uiFirstStop + psLine->uiNumStops - 1));
}


//////////////////////////////////////////////////////////////////////////
//
// The x of the row's last position.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextArea::GetRowRight(size uiRow)
{
	STextLayoutLine*	psLine;
	STextLayoutStop*	psStop;

	psLine = GetLayoutLine(uiRow);
	psStop = GetLayoutStop(psLine->uiFirstStop + psLine->uiNumStops - 1);
	if (IsRowWrapped(uiRow))
	{
		return psStop->x + psStop->iWidth;
	}
	return psStop->x;
}


//////////////////////////////////////////////////////////////////////////
//
// The row y (relative to the text area's top) is in.  Above the text it
// is the first, below it the last.  In a line's gap it is the row above.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindRowAt(int32 y)
{
	size				uiRow;
	size				uiNumRows;

	uiNumRows = NumLayoutLines();
	for (uiRow = 0; uiRow < uiNumRows - 1; uiRow++)
	{
		if (y < GetLayoutLine(uiRow + 1)->iTop)
		{
			break;
		}
	}
	return uiRow;
}


//////////////////////////////////////////////////////////////////////////
//
// The position on the row nearest x.  The end of a wrapped row is the
// start of the next with pbRowEnd true.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd)
{
	STextLayoutLine*	psLine;
	size				uiStop;
	size				uiEnd;
	size				uiBest;
	int32				iBestDistance;
	int32				iDistance;
	bool				bRowEnd;

	psLine = GetLayoutLine(uiRow);
	uiEnd = psLine->uiFirstStop + psLine->uiNumStops;
	uiBest = psLine->uiFirstStop;
	iBestDistance = abs(GetLayoutStop(uiBest)->x - x);
	for (uiStop = uiBest + 1; uiStop < uiEnd; uiStop++)
	{
		iDistance = abs(GetLayoutStop(uiStop)->x - x);
		if (iDistance < iBestDistance)
		{
			uiBest = uiStop;
			iBestDistance = iDistance;
		}
	}

	*pbRowEnd = false;
	if (IsRowWrapped(uiRow) && (abs(GetRowRight(uiRow) - x) < iBestDistance))
	{
		return GetRowEnd(uiRow, pbRowEnd);
	}
	if (!IsRowWrapped(uiRow) && (uiBest == uiEnd - 1))
	{
		//The last stop of a row that isn't wrapped is its line feed or the end of the text.
		return GetRowEnd(uiRow, &bRowEnd);
	}
	return *mauiStopStarts.Get(uiBest);
}


//////////////////////////////////////////////////////////////////////////
//
// The x of the caret at uiPosition, relative to the text area's left.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextArea::GetPositionX(size uiPosition, bool bRowEnd)
{
	size	uiStop;
	size	uiRow;

	uiStop = FindStop(uiPosition);
	uiRow = FindRow(uiPosition, bRowEnd);
	if (uiRow != GetLayoutStop(uiStop)->uiLine)
	{
		return GetRowRight(uiRow);
	}
	return GetLayoutStop(uiStop)->x;
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
// The width of a space, to show a line feed is selected.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextArea::GetNewLineWidth(void)
{
	Ptr<CFont>	pFont;

	pFont = GetFont();
	if (pFont.IsNull())
	{
		return 0;
	}
	return (int32)pFont->CalculateWidth((char*)" ");
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

	psLine = GetLayoutLine(uiRow);
	iTop = psLine->iTop;
	iMiddle = psLine->iTop + psLine->iHeight / 2;
	iPage = bDown ? GetPageHeight() : -GetPageHeight();

	uiPageRow = FindRowAt(iMiddle + iPage);
	ScrollBy(GetLayoutLine(uiPageRow)->iTop - iTop);
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


void CTextArea::UndoAction(void) { Undo(); }
void CTextArea::RedoAction(void) { Redo(); }


//////////////////////////////////////////////////////////////////////////
//
// The text is read and laid out again, the caret is kept in view and the
// window lays out again if the text's size changed.  Then the change
// event is called.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::TextEdited(void)
{
	SInt32Vec2	sTextSize;

	sTextSize = msTextSize;
	ReadText();
	LayoutText();
	if (((msTextSize.x != sTextSize.x) || (msTextSize.y != sTextSize.y)) && mpWindow.IsNotNull())
	{
		mpWindow->LayoutAgain();
	}
	ShowCaret();

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
	UpdateMarks();
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
// The selection is drawn as a range (AddRangeRectangles).  The caret is
// drawn by the window, blinking, over what it paints (GetCaretRect).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UpdateMarks(void)
{
	size		uiStart;
	size		uiEnd;

	//The selection is drawn with the text, so the text area draws again.
	mbMarksDirty = true;
	if (mpWindow.IsNotNull())
	{
		mpWindow->RequestPaint();
	}

	mpSelectionMap->Clear();
	if (!HasRows())
	{
		return;
	}

	if (GetSelection(&uiStart, &uiEnd))
	{
		AddRangeRectangles(mpSelectionMap, uiStart, uiEnd, msSelectionColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Draws the backgrounds into their map, each over the ones before it.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UpdateBackgrounds(void)
{
	size					i;
	STextAreaBackground*	psBackground;

	mpBackgroundMap->Clear();
	miBackgroundWidth = GetFullWidth();
	mbBackgroundsDirty = false;

	for (i = 0; i < mcBackgrounds.NumBackgrounds(); i++)
	{
		psBackground = mcBackgrounds.Get(i);
		if (psBackground->eType == TAB_Range)
		{
			if (HasRows())
			{
				AddRangeRectangles(mpBackgroundMap, psBackground->uiStart, psBackground->uiEnd, psBackground->sColour);
			}
		}
		else if (psBackground->eType == TAB_Lines)
		{
			AddLineBackgroundRectangles(psBackground);
		}
		else
		{
			AddBoundaryRectangle(psBackground);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One rectangle for each run of lines without gaps between them.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::AddLineBackgroundRectangles(STextAreaBackground* psBackground)
{
	size	uiLine;
	int32	iTop;
	int32	iBottom;
	int32	iRunTop;
	int32	iRunBottom;

	iRunTop = 0;
	iRunBottom = -1;
	for (uiLine = psBackground->uiStart; uiLine < psBackground->uiEnd; uiLine++)
	{
		if (!GetLineRows(uiLine, &iTop, &iBottom))
		{
			break;
		}
		if (iTop != iRunBottom)
		{
			if (iRunBottom > iRunTop)
			{
				mpBackgroundMap->AddRectangle(0, iRunTop, miBackgroundWidth, iRunBottom - iRunTop, psBackground->sColour);
			}
			iRunTop = iTop;
		}
		iRunBottom = iBottom;
	}
	if (iRunBottom > iRunTop)
	{
		mpBackgroundMap->AddRectangle(0, iRunTop, miBackgroundWidth, iRunBottom - iRunTop, psBackground->sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Centred on the line's top, but kept inside the text, so boundaries
// above the first line and below the last are whole.  A boundary with no
// line laid out is at the top.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::AddBoundaryRectangle(STextAreaBackground* psBackground)
{
	int32	iTop;
	int32	iBottom;
	int32	y;
	size	uiLast;

	y = 0;
	if (psBackground->uiStart < NumLines())
	{
		if (GetLineRows(psBackground->uiStart, &iTop, &iBottom))
		{
			y = iTop;
		}
	}
	else
	{
		uiLast = NumLines() - 1;
		if (GetLineRows(uiLast, &iTop, &iBottom))
		{
			y = iBottom + GetLineGap(uiLast);
		}
	}

	y -= psBackground->iThickness / 2;
	if (y + psBackground->iThickness > msTextSize.y)
	{
		y = msTextSize.y - psBackground->iThickness;
	}
	if (y < 0)
	{
		y = 0;
	}
	mpBackgroundMap->AddRectangle(0, y, miBackgroundWidth, psBackground->iThickness, psBackground->sColour);
}


//////////////////////////////////////////////////////////////////////////
//
// A block on each row uiStart to uiEnd is on, to the end of rows it goes
// past and a space further where they end at a line feed.  The text must
// have stops.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::AddRangeRectangles(Ptr<CGeometryMap> pMap, size uiStart, size uiEnd, ARGB32 sColour)
{
	size				uiRow;
	size				uiStartRow;
	size				uiEndRow;
	int32				iLeft;
	int32				iRight;
	STextLayoutLine*	psLine;

	uiStartRow = FindRow(uiStart, false);
	uiEndRow = FindRow(uiEnd, true);
	for (uiRow = uiStartRow; uiRow <= uiEndRow; uiRow++)
	{
		psLine = GetLayoutLine(uiRow);
		iLeft = (uiRow == uiStartRow) ? GetPositionX(uiStart, false) : 0;
		if (uiRow == uiEndRow)
		{
			iRight = GetPositionX(uiEnd, true);
		}
		else
		{
			iRight = GetRowRight(uiRow);
			if (!IsRowWrapped(uiRow))
			{
				iRight += GetNewLineWidth();
			}
		}
		if (iRight > iLeft)
		{
			pMap->AddRectangle(iLeft, psLine->iTop, iRight - iLeft, psLine->iHeight, sColour);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The top of line uiLine's first row and the bottom of its last, above
// its gap.  Text with no characters has its one line laid out without
// stops.  False if the line isn't laid out.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::GetLineRows(size uiLine, int32* piTop, int32* piBottom)
{
	STextLayoutLine*	psFirst;
	STextLayoutLine*	psLast;

	if (uiLine >= NumLines())
	{
		return false;
	}
	if (HasRows())
	{
		psFirst = GetLayoutLine(GetLayoutStop(FindStop(GetLineStart(uiLine)))->uiLine);
		psLast = GetLayoutLine(FindLastRow(uiLine));
	}
	else if ((uiLine == 0) && (NumLayoutLines() > 0))
	{
		psFirst = GetLayoutLine(0);
		psLast = psFirst;
	}
	else
	{
		return false;
	}

	*piTop = psFirst->iTop;
	*piBottom = psLast->iTop + psLast->iHeight;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The width of the laid out text, or of the text area if that is wider.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextArea::GetFullWidth(void)
{
	return (mcActualBounds.GetWidth() > msTextSize.x) ? mcActualBounds.GetWidth() : msTextSize.x;
}


bool CTextArea::IsEditable(void) { return mcEditing.IsEditable(); }
bool CTextArea::IsSelectable(void) { return mbSelectable; }
size CTextArea::GetCaretPosition(void) { return mcEditing.GetCaretPosition(); }


//////////////////////////////////////////////////////////////////////////
//
// Draws a fixed width font as tiles, each character in a cell of a tile
// layer, instead of as sprites.  Only used without word wrap.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetTiledText(bool bTiledText)
{
	if (mbTiledText != bTiledText)
	{
		mbTiledText = bTiledText;
		ReadText();
		LayoutText();

		//What it is laid out from has changed.
		InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::IsTiledText(void)
{
	return mbTiledText;
}


//////////////////////////////////////////////////////////////////////////
//
// Tiles were asked for and the font and word wrap allow them.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::IsDrawnAsTiles(void)
{
	return mbTiled;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::UseTiles(void)
{
	Ptr<CFont>	pFont;

	if (!mbTiledText || mbWordWrap || mpFamily.IsNull())
	{
		return false;
	}
	pFont = GetFont();
	return pFont.IsNotNull() && pFont->IsFixedWidth();
}


//////////////////////////////////////////////////////////////////////////
//
// The text read as tiles (CTextAreaTiles) in the colours of the spans,
// and its underlines, overlines and strike throughs drawn over them.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ReadTiles(void)
{
	STextStyle*	psStyles;

	//The sprites of the text read before are gone.
	mpText = NULL;

	psStyles = MakeStyles();
	mpTiles->Read(GetFontColours(), (int32)mcEditing.GetTabWidth(), psStyles, &msTextSize);

	//The lines along the text are drawn over the tiles, so they can go past a cell into the ones next to it.
	mpFontDraw->Begin();
	mpTiles->Decorate(mpFontDraw, GetFont()->GetDescription(), psStyles);
	mpFontDraw->End();
	SafeFree(psStyles);
}


//////////////////////////////////////////////////////////////////////////
//
// The text from uiFrom on (0 for all of it) in the fonts of its styles,
// its font changes from uiFrom.  NULL without a font for the text area's
// style.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CText> CTextArea::ReadTextFrom(size uiFrom)
{
	CSimpleTextReader		cReader;
	CUTF8					cUTF8;
	CArrayTextFontChange	asChanges;
	Ptr<CFont>				pFont;
	Ptr<CText>				pText;
	int16					iBaselineShift;

	pFont = mpFamily->GetRunFont(&msStyle, &iBaselineShift);
	if (pFont.IsNull())
	{
		return NULL;
	}

	//The reader's font has no baseline shift or decorations so a style with them needs a change at the start.
	asChanges.Init();
	if ((iBaselineShift != 0) || (msStyle.GetDecorations() != 0))
	{
		asChanges.Add()->Init(0, &pFont, iBaselineShift, msStyle.GetDecorations(), msStyle.sColour);
	}
	MakeFontChanges(&asChanges, pFont, iBaselineShift, uiFrom);

	cUTF8.Init(&mcDocument.Text()[uiFrom]);
	cReader.Init(&pFont);
	pText = cReader.Read(&cUTF8, true, &asChanges);
	cReader.Kill();
	cUTF8.Kill();
	asChanges.Kill();
	return pText;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether text appended can be laid out from the start of the old last
// line on rather than the whole text laid out again: it is laid out as
// sprites, up to date, at the width it is laid out to now, and nothing is
// decorated (decorations are drawn over the whole text).
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::CanAppendSprites(void)
{
	if (mbTiled || mbTextDirty || mbSizeDirty || mpText.IsNull() || UseTiles())
	{
		return false;
	}
	if ((mpLayout->GetWidth() != GetWrapWidth()) || (mpLayout->NumLines() == 0) || (mpLayout->NumStops() != mauiStopStarts.NumElements()))
	{
		return false;
	}
	return !HasDecorations();
}


//////////////////////////////////////////////////////////////////////////
//
// The text from uiFrom (the old end) on laid out after the lines before
// it: the old last line (from the line feed before it) and what was
// appended are read and laid out by themselves, from the top of that
// line, and replace its rows (CTextLayout::ReplaceLines) and their
// sprites.  CanAppendSprites says when.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::AppendSprites(size uiFrom)
{
	size				uiTextLine;
	size				uiTailStart;
	size				uiLayoutLine;
	size				uiSprite;
	int32				iTop;
	Ptr<CFont>			pFont;
	Ptr<CText>			pText;
	CArrayTextLayoutGap	asGaps;

	//The line the old end is on, and the row it starts.
	uiTextLine = mcDocument.NumLines() - 1;
	while ((uiTextLine > 0) && (mcDocument.GetLineStart(uiTextLine) > uiFrom))
	{
		uiTextLine--;
	}
	uiTailStart = mcDocument.GetLineStart(uiTextLine);
	uiLayoutLine = mpLayout->GetStop(FindStop(uiTailStart))->uiLine;
	iTop = mpLayout->GetLine(uiLayoutLine)->iTop;
	FindStopStarts(uiFrom);

	//Its rows' sprites go: they were placed after those of the rows above, below their tops.
	uiSprite = mpSpriteMap->NumSprites();
	while ((uiSprite > 0) && (mpSpriteMap->GetSprite(uiSprite - 1)->sPosition.y >= iTop))
	{
		uiSprite--;
	}
	mpSpriteMap->RemoveSprites(uiSprite);

	pText = ReadTextFrom(uiTailStart);
	if (mpAppendLayout.IsNull())
	{
		mpAppendLayout = OMalloc<CTextLayout>();
	}
	asGaps.Init();
	mcGaps.GetGapsFrom(uiTextLine, &asGaps);
	mpAppendLayout->SetWidth(mpLayout->GetWidth());
	mpAppendLayout->SetLineGaps(&asGaps);
	mpAppendLayout->SetTabWidth(GetTabPixels());
	asGaps.Kill();

	mpFontDraw->SetOffset(0, iTop);
	mpFontDraw->BeginAppend();
	if (pText.IsNotNull())
	{
		mpAppendLayout->Layout(mpFontDraw, pText);
	}
	mpFontDraw->End();
	mpFontDraw->SetOffset(0, 0);
	mpLayout->ReplaceLines(uiLayoutLine, &mpAppendLayout);
	pText = NULL;
	mbTextStale = true;

	msTextSize = mpLayout->GetSize();
	pFont = GetFont();
	if (pFont.IsNotNull() && (msTextSize.y < pFont->GetLineHeight()))
	{
		msTextSize.y = pFont->GetLineHeight();
	}
	mbBackgroundsDirty = true;
	UpdateMarks();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether text appended can be added to the tiles read before rather than
// the whole text read again: they are up to date, and nothing is decorated
// (decorations are drawn over the whole text).
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::CanAppendTiles(void)
{
	if (!mbTiled || mbTextDirty || mbSizeDirty || mpText.IsNotNull() || !UseTiles())
	{
		return false;
	}
	if ((mpTiles->NumStops() == 0) || (mpTiles->NumStops() != mauiStopStarts.NumElements()))
	{
		return false;
	}
	return !HasDecorations();
}


//////////////////////////////////////////////////////////////////////////
//
// The text from uiFrom (the old end) on added to the tiles, as ReadTiles
// would read it: its stops and lines after those before, the layer made
// big enough, and only the rows of its lines written (the first of them
// the old last line, which it may go on).  CanAppendTiles says when.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::AppendTiles(size uiFrom)
{
	STextStyle*	psStyles;

	FindStopStarts(uiFrom);
	psStyles = MakeStyles(mcDocument.GetLineStart(mpTiles->GetAppendLine()));
	mpTiles->Append(GetFontColours(), (int32)mcEditing.GetTabWidth(), psStyles, &msTextSize);
	SafeFree(psStyles);
}


//////////////////////////////////////////////////////////////////////////
//
// The tiles were laid out when they were read.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::LayoutTiles(void)
{
	Ptr<CFont>	pFont;

	pFont = GetFont();
	if (pFont.IsNotNull() && (msTextSize.y < pFont->GetLineHeight()))
	{
		msTextSize.y = pFont->GetLineHeight();
	}
	mbBackgroundsDirty = true;
	UpdateMarks();
}


//////////////////////////////////////////////////////////////////////////
//
// The lines and stops of the text as it was last laid out: by mpLayout,
// or from the columns when it is tiles.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::NumLayoutLines(void)
{
	return mbTiled ? mpTiles->NumLines() : mpLayout->NumLines();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::NumRows(void)
{
	return NumLayoutLines();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::NumLayoutStops(void)
{
	return mbTiled ? mpTiles->NumStops() : mpLayout->NumStops();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STextLayoutLine* CTextArea::GetLayoutLine(size uiLine)
{
	return mbTiled ? mpTiles->GetLine(uiLine) : mpLayout->GetLine(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
STextLayoutStop* CTextArea::GetLayoutStop(size uiStop)
{
	return mbTiled ? mpTiles->GetStop(uiStop) : mpLayout->GetStop(uiStop);
}

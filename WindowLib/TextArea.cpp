#include "BaseLib/UTF8.h"
#include "StandardLib/ClassDefines.h"
#include "MapsCanvasDraw.h"
#include "Window.h"
#include "NativeWindowFactory.h"
#include "JewelColours.h"
#include "TextArea.h"


static EComponentInputCommand	gaeTextAreaCommands[] =
{
	CIC_ActivateComponent, CIC_ExecuteComponent, CIC_StartDragSelect, CIC_ContDragSelect, CIC_StopDragSelect,
	CIC_Up, CIC_Down, CIC_Left, CIC_Right, CIC_WordLeft, CIC_WordRight, CIC_Home, CIC_End, CIC_DocumentHome, CIC_DocumentEnd,
	CIC_PageUp, CIC_PageDown, CIC_LineUp, CIC_LineDown,
	CIC_SelectUp, CIC_SelectDown, CIC_SelectLeft, CIC_SelectRight, CIC_SelectWordLeft, CIC_SelectWordRight, CIC_SelectHome,
	CIC_SelectEnd, CIC_SelectDocumentHome, CIC_SelectDocumentEnd, CIC_SelectPageUp, CIC_SelectPageDown, CIC_SelectWholeWord,
	CIC_SelectAll, CIC_ClearSelection,
	CIC_Space, CIC_Tab, CIC_BackTab, CIC_Enter, CIC_HomeEnter, CIC_EndEnter,
	CIC_Delete, CIC_Backspace, CIC_Cut, CIC_Copy, CIC_Paste, CIC_CutLine, CIC_Duplicate, CIC_DeleteWordRemainingLeft,
	CIC_DeleteWordRemainingRight, CIC_DeleteLine,
};


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  Line feeds start new lines.  pWhiteFont's image must
// be white (see CFontColours); the text area makes its own colours of it.
// It is the only style and size the text area has.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CFont> pWhiteFont, const char* szText)
{
	Init(pWindow, OMalloc<CFontColours>(pWhiteFont), szText);
}


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  pColours can be shared by text areas so each colour is
// only made once.  It is the only style and size the text area has.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CFontColours> pColours, const char* szText)
{
	Init(pWindow, OMalloc<CFontFamily>(pColours), szText);
}


//////////////////////////////////////////////////////////////////////////
//
// szText is UTF8.  Line feeds start new lines.  pFamily can be shared by
// text areas.  The caret starts at the start of the text, and the text
// area is editable.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Init(Ptr<CWindow> pWindow, Ptr<CFontFamily> pFamily, const char* szText)
{
	Ptr<CMapsCanvasDraw>	pDraw;

	PreInit();

	mszText.Init(szText);
	mpFamily = pFamily;
	msStyle.Init();
	masSpans.Init();
	mbTextDirty = false;
	mbSizeDirty = false;
	mpText = NULL;
	mpLayout = OMalloc<CTextLayout>();
	mpSpriteMap = OMalloc<CSpriteMap>();
	mpGeometryMap = OMalloc<CGeometryMap>();
	mpFontDraw = OMalloc<CMapFontDraw>(mpSpriteMap, mpGeometryMap);
	mpSelectionMap = OMalloc<CGeometryMap>();
	mpCaretMap = OMalloc<CGeometryMap>();
	mpMaps = OMalloc<CMaps>();
	mpMaps->AddMap(mpSelectionMap);
	mpMaps->AddMap(mpSpriteMap);
	mpMaps->AddMap(mpGeometryMap);
	mpMaps->AddMap(mpCaretMap);
	mpMaps->SetViewportPosition(0, 0);
	mbWordWrap = false;
	msTextSize.Init(0, 0);
	msSelectionColour = JEWEL_SELECTION;
	msCaretColour = JEWEL_ICON;
	mauiStopStarts.Init();
	muiCaret = 0;
	muiAnchor = MAX_SIZE;
	mbCaretAtRowEnd = false;
	miUpDownX = -1;
	miTabWidth = 4;
	mbEditable = true;
	mbFocussed = false;
	mbDragging = false;

	//The text is drawn over whatever is behind the text area.
	pDraw = OMalloc<CMapsCanvasDraw>(mpMaps);
	CCanvas::Init(pWindow, pDraw);
	SetTransparent(true);
	SetCanGetFocus(true);

	ReadText();
	LayoutText();

	AddInputActions(gaeTextAreaCommands, sizeof(gaeTextAreaCommands) / sizeof(EComponentInputCommand), this, &CTextArea::CommandAction);
	AddInputAction(IAF_Printable, NULL, this, &CTextArea::PrintableAction);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Free(void)
{
	mauiStopStarts.Kill();
	masSpans.Kill();
	mszText.Kill();
	CCanvas::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Class(void)
{
	CCanvas::Class();

	U_String(mszText);
	M_Pointer(mpFamily);
	U_Data(STextStyle, msStyle);
	U_Data(CArrayTextAreaSpan, masSpans);
	U_Bool(mbTextDirty);
	U_Bool(mbSizeDirty);
	M_Pointer(mpText);
	M_Pointer(mpLayout);
	M_Pointer(mpSpriteMap);
	M_Pointer(mpGeometryMap);
	M_Pointer(mpFontDraw);
	M_Pointer(mpMaps);
	U_Bool(mbWordWrap);
	U_2Int32(msTextSize);
	M_Pointer(mpSelectionMap);
	M_Pointer(mpCaretMap);
	U_UInt32(msSelectionColour);
	U_UInt32(msCaretColour);
	U_Data(CArrayTextAreaStopStart, mauiStopStarts);
	U_Size(muiCaret);
	U_Size(muiAnchor);
	U_Bool(mbCaretAtRowEnd);
	U_Int32(miUpDownX);
	U_Int32(miTabWidth);
	U_Bool(mbEditable);
	U_Bool(mbFocussed);
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
// The spans are removed; they were ranges of the old text.  The caret goes
// to the start and nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetText(const char* szText)
{
	mszText.Kill();
	mszText.Init(szText);
	masSpans.ReInit();
	muiCaret = 0;
	muiAnchor = MAX_SIZE;
	mbCaretAtRowEnd = false;
	miUpDownX = -1;
	ReadText();
	LayoutText();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds szText (UTF8) to the end of the text in sColour.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::AppendText(const char* szText, ARGB32 sColour)
{
	size	uiStart;

	uiStart = mszText.Length();
	mszText.Append(szText);
	AddSpan(uiStart, mszText.Length() - uiStart, sColour);
	ReadText();
	LayoutText();
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
		LayoutText();
	}
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
	size			uiTextLength;
	STextAreaSpan*	psSpan;

	uiTextLength = mszText.Length();
	if ((uiStart > uiTextLength) || (uiLength > uiTextLength - uiStart))
	{
		return false;
	}
	if ((uiLength == 0) || (psOverride->uiSet == 0))
	{
		return true;
	}

	psSpan = masSpans.Add();
	psSpan->uiStart = uiStart;
	psSpan->uiLength = uiLength;
	psSpan->sOverride = *psOverride;
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
	size	i;

	for (i = 0; i < masSpans.NumElements(); i++)
	{
		if (masSpans.Get(i)->sOverride.ChangesFont())
		{
			mbSizeDirty = true;
		}
	}
	if (masSpans.NumElements() > 0)
	{
		masSpans.ReInit();
		mbTextDirty = true;
	}
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
	size			i;
	STextAreaSpan*	psSpan;

	*psStyle = msStyle;
	for (i = 0; i < masSpans.NumElements(); i++)
	{
		psSpan = masSpans.Get(i);
		if ((uiPosition >= psSpan->uiStart) && (uiPosition < psSpan->uiStart + psSpan->uiLength))
		{
			psSpan->sOverride.ApplyTo(psStyle);
		}
	}
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

	if (mbTextDirty || (mpLayout->GetWidth() != GetWrapWidth()))
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
// Spans or a style set since the text area was last laid out are drawn.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::Draw(void)
{
	UpdateText();
	return CCanvas::Draw();
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
	CSimpleTextReader		cReader;
	CUTF8					cUTF8;
	CArrayTextFontChange	asChanges;
	Ptr<CFont>				pFont;
	int16					iBaselineShift;

	mbTextDirty = false;
	mbSizeDirty = false;
	FindStopStarts();

	pFont = mpFamily->GetRunFont(&msStyle, &iBaselineShift);
	if (pFont.IsNull())
	{
		mpText = NULL;
		return;
	}

	//The reader's font has no baseline shift or decorations so a style with them needs a change at the start.
	asChanges.Init();
	if ((iBaselineShift != 0) || (msStyle.GetDecorations() != 0))
	{
		asChanges.Add()->Init(0, &pFont, iBaselineShift, msStyle.GetDecorations(), msStyle.sColour);
	}
	MakeFontChanges(&asChanges, pFont, iBaselineShift);

	cUTF8.Init(mszText.Text());
	cReader.Init(&pFont);
	mpText = cReader.Read(&cUTF8, true, &asChanges);
	cReader.Kill();
	cUTF8.Kill();
	asChanges.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each byte of the text is given the text area's style with the spans
// that cover it applied in the order they were added, then a font change
// is made wherever the style changes.  Text before the first change is in
// pDefaultFont.  A style the family has no font for is drawn in
// pDefaultFont (the family logs an error).
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::MakeFontChanges(CArrayTextFontChange* pasChanges, Ptr<CFont> pDefaultFont, int16 iDefaultBaselineShift)
{
	size				uiTextLength;
	size				i;
	size				uiPosition;
	size				uiEnd;
	STextStyle*			psStyles;
	STextAreaSpan*		psSpan;
	STextStyle*			psCurrent;
	Ptr<CFont>			pFont;
	int16				iBaselineShift;

	uiTextLength = mszText.Length();
	if ((masSpans.NumElements() == 0) || (uiTextLength == 0))
	{
		return;
	}

	psStyles = (STextStyle*)malloc(uiTextLength * sizeof(STextStyle));
	for (uiPosition = 0; uiPosition < uiTextLength; uiPosition++)
	{
		psStyles[uiPosition] = msStyle;
	}
	for (i = 0; i < masSpans.NumElements(); i++)
	{
		psSpan = masSpans.Get(i);
		uiEnd = psSpan->uiStart + psSpan->uiLength;
		for (uiPosition = psSpan->uiStart; uiPosition < uiEnd; uiPosition++)
		{
			psSpan->sOverride.ApplyTo(&psStyles[uiPosition]);
		}
	}

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
	free(psStyles);
}


//////////////////////////////////////////////////////////////////////////
//
// Places a sprite for each glyph, wrapped to the actual width if word wrap
// is on, and works out the size they need.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::LayoutText(void)
{
	Ptr<CFont>	pFont;

	mpLayout->SetWidth(GetWrapWidth());

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

	//Even a text area with no text is a line high.
	pFont = GetFont();
	if (pFont.IsNotNull() && (msTextSize.y < pFont->GetLineHeight()))
	{
		msTextSize.y = pFont->GetLineHeight();
	}

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


char* CTextArea::GetText(void) { return mszText.Text(); }
Ptr<CFontFamily> CTextArea::GetFontFamily(void) { return mpFamily; }
bool CTextArea::IsWordWrap(void) { return mbWordWrap; }
ARGB32 CTextArea::GetColour(void) { return msStyle.sColour; }
size CTextArea::NumSpans(void) { return masSpans.NumElements(); }



//////////////////////////////////////////////////////////////////////////
//
// A text area that isn't editable can still be navigated, selected and
// copied from.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetEditable(bool bEditable)
{
	mbEditable = bEditable;
}


//////////////////////////////////////////////////////////////////////////
//
// How many spaces apart the tab stops are.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SetTabWidth(int32 iTabWidth)
{
	if (iTabWidth < 1)
	{
		iTabWidth = 1;
	}
	miTabWidth = iTabWidth;
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
	if (uiPosition > mszText.Length())
	{
		uiPosition = mszText.Length();
	}
	MoveCaret(uiPosition, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if nothing is selected, then the start and end are the
// caret.  The start is before the end.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::GetSelection(size* puiStart, size* puiEnd)
{
	if (!IsSelected())
	{
		*puiStart = muiCaret;
		*puiEnd = muiCaret;
		return false;
	}
	if (muiAnchor < muiCaret)
	{
		*puiStart = muiAnchor;
		*puiEnd = muiCaret;
	}
	else
	{
		*puiStart = muiCaret;
		*puiEnd = muiAnchor;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Selects from uiStart to uiEnd with the caret at uiEnd.  The same start
// and end selects nothing.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Select(size uiStart, size uiEnd)
{
	if (uiStart > mszText.Length())
	{
		uiStart = mszText.Length();
	}
	if (uiEnd > mszText.Length())
	{
		uiEnd = mszText.Length();
	}
	muiAnchor = uiStart;
	MoveCaret(uiEnd, true);
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

	if (!HasStops())
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

	if (!HasStops())
	{
		pcDest->Init(0, 0, 1, msTextSize.y);
		return;
	}

	psLine = mpLayout->GetLine(GetCaretRow());
	x = GetCaretX();
	if ((mcActualBounds.GetWidth() > 0) && (x > mcActualBounds.GetWidth() - 1))
	{
		x = mcActualBounds.GetWidth() - 1;
	}
	pcDest->Init(x, psLine->iTop, x + 1, psLine->iTop + psLine->iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// Navigation.  Moving without selecting clears the selection.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Up(void) { MoveUpDown(false, false, false); }
void CTextArea::Down(void) { MoveUpDown(true, false, false); }
void CTextArea::Left(void) { MoveCaret(PreviousPosition(muiCaret), false); }
void CTextArea::Right(void) { MoveCaret(NextPosition(muiCaret), false); }
void CTextArea::WordLeft(void) { MoveCaret(FindWordLeft(muiCaret), false); }
void CTextArea::WordRight(void) { MoveCaret(FindWordRight(muiCaret), false); }
void CTextArea::Home(void) { MoveHome(false); }
void CTextArea::End(void) { MoveEnd(false); }
void CTextArea::DocumentHome(void) { MoveCaret(0, false); }
void CTextArea::DocumentEnd(void) { MoveCaret(mszText.Length(), false); }
void CTextArea::PageUp(void) { MoveUpDown(false, true, false); }
void CTextArea::PageDown(void) { MoveUpDown(true, true, false); }


//////////////////////////////////////////////////////////////////////////
//
// Selection.  The selection is from where it started to the caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SelectUp(void) { MoveUpDown(false, false, true); }
void CTextArea::SelectDown(void) { MoveUpDown(true, false, true); }
void CTextArea::SelectLeft(void) { MoveCaret(PreviousPosition(muiCaret), true); }
void CTextArea::SelectRight(void) { MoveCaret(NextPosition(muiCaret), true); }
void CTextArea::SelectWordLeft(void) { MoveCaret(FindWordLeft(muiCaret), true); }
void CTextArea::SelectWordRight(void) { MoveCaret(FindWordRight(muiCaret), true); }
void CTextArea::SelectHome(void) { MoveHome(true); }
void CTextArea::SelectEnd(void) { MoveEnd(true); }
void CTextArea::SelectDocumentHome(void) { MoveCaret(0, true); }
void CTextArea::SelectDocumentEnd(void) { MoveCaret(mszText.Length(), true); }
void CTextArea::SelectPageUp(void) { MoveUpDown(false, true, true); }
void CTextArea::SelectPageDown(void) { MoveUpDown(true, true, true); }
void CTextArea::SelectAll(void) { Select(0, mszText.Length()); }


//////////////////////////////////////////////////////////////////////////
//
// Selects the word the caret is in or next to.  Not next to a word,
// nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::SelectWholeWord(void)
{
	size	uiStart;
	size	uiEnd;
	char*	szText;

	szText = mszText.Text();
	uiStart = muiCaret;
	if ((muiCaret > 0) && IsWordChar(szText[PreviousPosition(muiCaret)]))
	{
		uiStart = FindWordLeft(muiCaret);
	}
	uiEnd = muiCaret;
	if ((muiCaret < mszText.Length()) && IsWordChar(szText[muiCaret]))
	{
		uiEnd = FindWordRight(muiCaret);
	}
	Select(uiStart, uiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ClearSelection(void)
{
	muiAnchor = MAX_SIZE;
	UpdateMarks();
}


//////////////////////////////////////////////////////////////////////////
//
// c replaces the selection.  Without one it is inserted, or with bInsert
// false it overwrites the character after the caret unless that ends the
// line.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Printable(char c, bool bInsert)
{
	char	sz[2];
	char*	szText;

	if (!mbEditable)
	{
		return;
	}

	sz[0] = c;
	sz[1] = '\0';
	szText = mszText.Text();
	if (!IsSelected() && !bInsert && (muiCaret < mszText.Length()) && (szText[muiCaret] != '\n') && (szText[muiCaret] != '\r'))
	{
		RemoveText(muiCaret, NextPosition(muiCaret));
	}
	ReplaceSelection(sz);
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Space(bool bInsert)
{
	Printable(' ', bInsert);
}


//////////////////////////////////////////////////////////////////////////
//
// Spaces to the next tab stop replace the selection, or are inserted.  A
// selection over more than one line has its lines indented instead.
// bInsert is ignored, tabs never overwrite.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Tab(bool bInsert)
{
	size	uiStart;
	size	uiEnd;
	size	uiColumns;
	CChars	szSpaces;

	if (!mbEditable)
	{
		return;
	}

	GetSelection(&uiStart, &uiEnd);
	if (FindLineStart(uiStart) != FindLineStart(uiEnd))
	{
		IndentLines(uiStart, uiEnd);
	}
	else
	{
		uiColumns = CountColumns(FindLineStart(uiStart), uiStart);
		szSpaces.Init();
		szSpaces.Append(' ', miTabWidth - (uiColumns % miTabWidth));
		ReplaceSelection(szSpaces.Text());
		szSpaces.Kill();
	}
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Removes a tab's worth of spaces, or a tab, from the start of the lines
// the selection is over, or the caret's line.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::BackTab(void)
{
	size	uiStart;
	size	uiEnd;

	if (!mbEditable)
	{
		return;
	}

	GetSelection(&uiStart, &uiEnd);
	OutdentLines(uiStart, uiEnd);
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// A line feed replaces the selection, or is inserted.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Enter(void)
{
	if (!mbEditable)
	{
		return;
	}

	ReplaceSelection("\n");
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Starts a new line before the caret's line, and the caret goes to it.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::HomeEnter(void)
{
	size	uiLineStart;

	if (!mbEditable)
	{
		return;
	}

	muiAnchor = MAX_SIZE;
	uiLineStart = FindLineStart(muiCaret);
	InsertText(uiLineStart, "\n");
	muiCaret = uiLineStart;
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Starts a new line after the caret's line, and the caret goes to it.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::EndEnter(void)
{
	if (!mbEditable)
	{
		return;
	}

	muiAnchor = MAX_SIZE;
	muiCaret = FindLineEnd(muiCaret);
	InsertText(muiCaret, "\n");
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes the selection, or the character after the caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Delete(void)
{
	if (!mbEditable)
	{
		return;
	}

	if (!DeleteSelection())
	{
		RemoveText(muiCaret, NextPosition(muiCaret));
	}
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes the selection, or the character before the caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Backspace(void)
{
	if (!mbEditable)
	{
		return;
	}

	if (!DeleteSelection())
	{
		RemoveText(PreviousPosition(muiCaret), muiCaret);
	}
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// The selection is appended to pszDest.  Without one nothing is copied.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Copy(CChars* pszDest)
{
	size	uiStart;
	size	uiEnd;

	if (GetSelection(&uiStart, &uiEnd))
	{
		pszDest->AppendSubString(mszText, uiStart, uiEnd);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The selection is appended to pszDest and deleted.  Without one the
// caret's line is cut.  A text area that isn't editable only copies.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Cut(CChars* pszDest)
{
	if (!mbEditable)
	{
		Copy(pszDest);
		return;
	}

	if (IsSelected())
	{
		Copy(pszDest);
		DeleteSelection();
		TextChanged();
	}
	else
	{
		CutLine(pszDest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// szSource replaces the selection, or is inserted.  Its carriage returns
// become line feeds, or go where they come before one.  Pasting nothing
// leaves a selection where it is.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Paste(char* szSource)
{
	CChars	szText;

	if (!mbEditable || (szSource == NULL) || (szSource[0] == '\0'))
	{
		return;
	}

	szText.Init(szSource);
	szText.Replace("\r\n", "\n");
	szText.Replace('\r', '\n');
	ReplaceSelection(szText.Text());
	szText.Kill();
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// The caret's line, with its line feed, is appended to pszDest and
// deleted.  The caret stays as far along the line that moves up into its
// place.  With a selection only the selection is cut.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::CutLine(CChars* pszDest)
{
	size	uiLineStart;
	size	uiEnd;
	size	uiColumn;

	if (!mbEditable)
	{
		return;
	}
	if (IsSelected())
	{
		Cut(pszDest);
		return;
	}

	uiLineStart = FindLineStart(muiCaret);
	uiColumn = CountColumns(uiLineStart, muiCaret);
	uiEnd = NextPosition(FindLineEnd(muiCaret));
	pszDest->AppendSubString(mszText, uiLineStart, uiEnd);
	RemoveText(uiLineStart, uiEnd);
	muiCaret = FindColumn(uiLineStart, uiColumn);
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// The selection is copied in front of itself and the copy after it stays
// selected.  Without one the caret's line is copied above itself and the
// caret goes with the line below.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::Duplicate(void)
{
	size	uiStart;
	size	uiEnd;
	CChars	szCopy;

	if (!mbEditable)
	{
		return;
	}

	szCopy.Init();
	if (GetSelection(&uiStart, &uiEnd))
	{
		szCopy.AppendSubString(mszText, uiStart, uiEnd);
	}
	else
	{
		uiStart = FindLineStart(muiCaret);
		uiEnd = FindLineEnd(muiCaret);
		szCopy.AppendSubString(mszText, uiStart, uiEnd);
		szCopy.Append('\n');
	}
	InsertText(uiStart, szCopy.Text());
	szCopy.Kill();
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes from the caret to the end of the word, or the spaces, it is
// in.  A selection is deleted instead.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::DeleteWordRemainingRight(void)
{
	if (!mbEditable)
	{
		return;
	}

	if (!DeleteSelection())
	{
		RemoveText(muiCaret, FindWordRight(muiCaret));
	}
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes from the start of the word, or the spaces, the caret is in to
// the caret.  A selection is deleted instead.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::DeleteWordRemainingLeft(void)
{
	if (!mbEditable)
	{
		return;
	}

	if (!DeleteSelection())
	{
		RemoveText(FindWordLeft(muiCaret), muiCaret);
	}
	TextChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes the lines the selection is over, or the caret's line, with
// their line feed.  The last line takes the line feed before it.  The
// caret stays as far along the line it is left on.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::DeleteLine(void)
{
	size	uiStart;
	size	uiEnd;
	size	uiColumn;
	bool	bSelected;

	if (!mbEditable)
	{
		return;
	}

	uiColumn = CountColumns(FindLineStart(muiCaret), muiCaret);
	bSelected = GetSelection(&uiStart, &uiEnd);
	uiStart = FindLineStart(uiStart);
	if (bSelected && (uiEnd > uiStart) && (FindLineStart(uiEnd) == uiEnd))
	{
		//A selection that ends at the start of a line doesn't take that line.
		uiEnd = PreviousPosition(uiEnd);
	}
	uiEnd = FindLineEnd(uiEnd);
	if (uiEnd < mszText.Length())
	{
		uiEnd = NextPosition(uiEnd);
	}
	else if (uiStart > 0)
	{
		uiStart = PreviousPosition(uiStart);
	}

	muiAnchor = MAX_SIZE;
	RemoveText(uiStart, uiEnd);
	muiCaret = FindColumn(FindLineStart(uiStart), uiColumn);
	TextChanged();
}


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
	return mbEditable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::FocusGained(void)
{
	mbFocussed = true;
	UpdateMarks();
}


//////////////////////////////////////////////////////////////////////////
//
// Losing focus, as when the window is deactivated, ends a drag.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::FocusLost(void)
{
	mbFocussed = false;
	mbDragging = false;
	UpdateMarks();
}


//////////////////////////////////////////////////////////////////////////
//
// The input actions of gaeTextAreaCommands.  The clipboard is the native
// window factory's; cutting or copying nothing leaves it as it was.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::CommandAction(SComponentInputEvent* psEvent)
{
	CChars	szClipboard;

	switch (psEvent->eCommand)
	{
	case CIC_ActivateComponent:			ActivateComponent(psEvent->x, psEvent->y);	break;
	case CIC_ExecuteComponent:			ExecuteComponent(psEvent->x, psEvent->y);	break;
	case CIC_StartDragSelect:			StartDragSelect(psEvent->x, psEvent->y);	break;
	case CIC_ContDragSelect:			ContDragSelect(psEvent->x, psEvent->y);		break;
	case CIC_StopDragSelect:			StopDragSelect(psEvent->x, psEvent->y);		break;
	case CIC_Up:						Up();						break;
	case CIC_Down:						Down();						break;
	case CIC_Left:						Left();						break;
	case CIC_Right:						Right();					break;
	case CIC_WordLeft:					WordLeft();					break;
	case CIC_WordRight:					WordRight();				break;
	case CIC_Home:						Home();						break;
	case CIC_End:						End();						break;
	case CIC_DocumentHome:				DocumentHome();				break;
	case CIC_DocumentEnd:				DocumentEnd();				break;
	case CIC_PageUp:					PageUp();					break;
	case CIC_PageDown:					PageDown();					break;
	case CIC_LineUp:					LineUp();					break;
	case CIC_LineDown:					LineDown();					break;
	case CIC_SelectUp:					SelectUp();					break;
	case CIC_SelectDown:				SelectDown();				break;
	case CIC_SelectLeft:				SelectLeft();				break;
	case CIC_SelectRight:				SelectRight();				break;
	case CIC_SelectWordLeft:			SelectWordLeft();			break;
	case CIC_SelectWordRight:			SelectWordRight();			break;
	case CIC_SelectHome:				SelectHome();				break;
	case CIC_SelectEnd:					SelectEnd();				break;
	case CIC_SelectDocumentHome:		SelectDocumentHome();		break;
	case CIC_SelectDocumentEnd:			SelectDocumentEnd();		break;
	case CIC_SelectPageUp:				SelectPageUp();				break;
	case CIC_SelectPageDown:			SelectPageDown();			break;
	case CIC_SelectWholeWord:			SelectWholeWord();			break;
	case CIC_SelectAll:					SelectAll();				break;
	case CIC_ClearSelection:			ClearSelection();			break;
	case CIC_Space:						Space(psEvent->bInsert);	break;
	case CIC_Tab:						Tab(psEvent->bInsert);		break;
	case CIC_BackTab:					BackTab();					break;
	case CIC_Enter:						Enter();					break;
	case CIC_HomeEnter:					HomeEnter();				break;
	case CIC_EndEnter:					EndEnter();					break;
	case CIC_Delete:					Delete();					break;
	case CIC_Backspace:					Backspace();				break;
	case CIC_Duplicate:					Duplicate();				break;
	case CIC_DeleteWordRemainingRight:	DeleteWordRemainingRight();	break;
	case CIC_DeleteWordRemainingLeft:	DeleteWordRemainingLeft();	break;
	case CIC_DeleteLine:				DeleteLine();				break;
	case CIC_Cut:
	case CIC_Copy:
	case CIC_CutLine:
		szClipboard.Init();
		if (psEvent->eCommand == CIC_Cut)
		{
			Cut(&szClipboard);
		}
		else if (psEvent->eCommand == CIC_CutLine)
		{
			CutLine(&szClipboard);
		}
		else
		{
			Copy(&szClipboard);
		}
		if (!szClipboard.Empty())
		{
			mpWindow->GetFactory()->SetClipboardText(szClipboard.Text());
		}
		szClipboard.Kill();
		break;
	case CIC_Paste:
		szClipboard.Init();
		mpWindow->GetFactory()->GetClipboardText(&szClipboard);
		Paste(szClipboard.Text());
		szClipboard.Kill();
		break;
	default:
		break;
	}
}


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
	if (HasStops())
	{
		uiPosition = FindPositionInRow(FindRowAt(y), x, &bRowEnd);
	}
	MoveCaret(uiPosition, false, bRowEnd);
	mpWindow->GetFocus()->StartDrag(FD_Select, x, y);
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
	if (!IsSelected())
	{
		muiAnchor = muiCaret;
	}
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

	if (mbDragging && HasStops())
	{
		uiPosition = FindPositionInRow(FindRowAt(y), x, &bRowEnd);
		MoveCaret(uiPosition, true, bRowEnd);
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
// is read: a stop for each character but carriage returns, then one at
// the end of the text.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::FindStopStarts(void)
{
	size	uiPosition;
	size	uiLength;
	char*	szText;

	mauiStopStarts.ReInit();
	szText = mszText.Text();
	uiLength = mszText.Length();
	uiPosition = 0;
	while (uiPosition < uiLength)
	{
		if (szText[uiPosition] != '\r')
		{
			*mauiStopStarts.Add() = uiPosition;
		}
		uiPosition += CharacterLength(uiPosition);
	}
	*mauiStopStarts.Add() = uiLength;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes in the character at uiPosition: a code point, or a sequence
// of them joined by ZWJs.  A byte that doesn't start valid UTF8 is a
// character by itself.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::CharacterLength(size uiPosition)
{
	CUTF8	cUTF8;
	size	uiRemaining;
	size	uiCharLength;

	if (uiPosition >= mszText.Length())
	{
		return 0;
	}

	uiRemaining = mszText.Length() - uiPosition;
	cUTF8.Init((uint8*)mszText.Text() + uiPosition, uiRemaining);
	uiCharLength = cUTF8.PeekUTFBytes();
	cUTF8.Kill();
	if ((uiCharLength == 0) || (uiCharLength > uiRemaining))
	{
		return 1;
	}
	return uiCharLength;
}


//////////////////////////////////////////////////////////////////////////
//
// The start of the character after the one at uiPosition.  A carriage
// return and line feed are one character.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::NextPosition(size uiPosition)
{
	char*	szText;

	if (uiPosition >= mszText.Length())
	{
		return mszText.Length();
	}

	szText = mszText.Text();
	if ((szText[uiPosition] == '\r') && (szText[uiPosition + 1] == '\n'))
	{
		return uiPosition + 2;
	}
	return uiPosition + CharacterLength(uiPosition);
}


//////////////////////////////////////////////////////////////////////////
//
// The start of the character before uiPosition.  Characters can only be
// found going forward, so they are from the start of the line.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::PreviousPosition(size uiPosition)
{
	size	uiCurrent;
	size	uiNext;
	char*	szText;

	if (uiPosition == 0)
	{
		return 0;
	}

	szText = mszText.Text();
	uiCurrent = FindLineStart(uiPosition);
	if (uiCurrent == uiPosition)
	{
		if ((uiPosition >= 2) && (szText[uiPosition - 2] == '\r'))
		{
			return uiPosition - 2;
		}
		return uiPosition - 1;
	}

	for (;;)
	{
		uiNext = NextPosition(uiCurrent);
		if (uiNext >= uiPosition)
		{
			return uiCurrent;
		}
		uiCurrent = uiNext;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The start of the line (ended by line feeds) uiPosition is on.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindLineStart(size uiPosition)
{
	char*	szText;

	szText = mszText.Text();
	while ((uiPosition > 0) && (szText[uiPosition - 1] != '\n'))
	{
		uiPosition--;
	}
	return uiPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// The end of the line uiPosition is on, before its carriage return and
// line feed, or the end of the text.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindLineEnd(size uiPosition)
{
	char*	szText;
	size	uiLength;
	size	uiStart;

	szText = mszText.Text();
	uiLength = mszText.Length();
	uiStart = uiPosition;
	while ((uiPosition < uiLength) && (szText[uiPosition] != '\n'))
	{
		uiPosition++;
	}
	if ((uiPosition < uiLength) && (uiPosition > uiStart) && (szText[uiPosition - 1] == '\r'))
	{
		uiPosition--;
	}
	return uiPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// The first character of the line that isn't a space or tab, or the end
// of the line.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindLineIndent(size uiLineStart)
{
	size	uiLineEnd;
	char*	szText;

	szText = mszText.Text();
	uiLineEnd = FindLineEnd(uiLineStart);
	while ((uiLineStart < uiLineEnd) && ((szText[uiLineStart] == ' ') || (szText[uiLineStart] == '\t')))
	{
		uiLineStart++;
	}
	return uiLineStart;
}


//////////////////////////////////////////////////////////////////////////
//
// The number of characters from the start of the line to uiPosition.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::CountColumns(size uiLineStart, size uiPosition)
{
	size	uiColumns;

	uiColumns = 0;
	while (uiLineStart < uiPosition)
	{
		uiLineStart = NextPosition(uiLineStart);
		uiColumns++;
	}
	return uiColumns;
}


//////////////////////////////////////////////////////////////////////////
//
// The position uiColumn characters along the line, or its end if it is
// shorter.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindColumn(size uiLineStart, size uiColumn)
{
	size	uiLineEnd;

	uiLineEnd = FindLineEnd(uiLineStart);
	while ((uiColumn > 0) && (uiLineStart < uiLineEnd))
	{
		uiLineStart = NextPosition(uiLineStart);
		uiColumn--;
	}
	return uiLineStart;
}


//////////////////////////////////////////////////////////////////////////
//
// The start of the word, or spaces, before uiPosition.  Punctuation is a
// word of one character.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindWordLeft(size uiPosition)
{
	size	uiPrevious;
	char*	szText;
	bool	bWhitespace;

	if (uiPosition == 0)
	{
		return 0;
	}

	szText = mszText.Text();
	uiPosition = PreviousPosition(uiPosition);
	bWhitespace = IsWhitespace(szText[uiPosition]);
	if (!bWhitespace && !IsWordChar(szText[uiPosition]))
	{
		return uiPosition;
	}

	while (uiPosition > 0)
	{
		uiPrevious = PreviousPosition(uiPosition);
		if (bWhitespace ? !IsWhitespace(szText[uiPrevious]) : !IsWordChar(szText[uiPrevious]))
		{
			break;
		}
		uiPosition = uiPrevious;
	}
	return uiPosition;
}


//////////////////////////////////////////////////////////////////////////
//
// The end of the word, or spaces, after uiPosition.  Punctuation is a
// word of one character.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindWordRight(size uiPosition)
{
	size	uiLength;
	char*	szText;
	bool	bWhitespace;

	uiLength = mszText.Length();
	if (uiPosition >= uiLength)
	{
		return uiLength;
	}

	szText = mszText.Text();
	bWhitespace = IsWhitespace(szText[uiPosition]);
	if (!bWhitespace && !IsWordChar(szText[uiPosition]))
	{
		return NextPosition(uiPosition);
	}

	while ((uiPosition < uiLength) && (bWhitespace ? IsWhitespace(szText[uiPosition]) : IsWordChar(szText[uiPosition])))
	{
		uiPosition = NextPosition(uiPosition);
	}
	return uiPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::IsWhitespace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
}


//////////////////////////////////////////////////////////////////////////
//
// Letters, digits and underscores.  Any character that isn't ASCII is
// treated as a letter.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::IsWordChar(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_') || ((uint8)c >= 0x80);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the layout is of the text as it is.  Without a font there is no
// layout, and the caret and selection aren't drawn.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::HasStops(void)
{
	return mpText.IsNotNull() && (mpLayout->NumLines() > 0) && (mpLayout->NumStops() == mauiStopStarts.NumElements());
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

	if (uiRow + 1 >= mpLayout->NumLines())
	{
		return false;
	}
	psLine = mpLayout->GetLine(uiRow);
	uiLast = psLine->uiFirstStop + psLine->uiNumStops - 1;
	return mszText.GetChar(*mauiStopStarts.Get(uiLast)) != '\n';
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

	uiRow = mpLayout->GetStop(FindStop(uiPosition))->uiLine;
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
	return *mauiStopStarts.Get(mpLayout->GetLine(uiRow)->uiFirstStop);
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
	if (uiRow + 1 >= mpLayout->NumLines())
	{
		return mszText.Length();
	}

	psLine = mpLayout->GetLine(uiRow);
	if (IsRowWrapped(uiRow))
	{
		*pbRowEnd = true;
		return *mauiStopStarts.Get(psLine->uiFirstStop + psLine->uiNumStops);
	}
	return FindLineEnd(*mauiStopStarts.Get(psLine->uiFirstStop + psLine->uiNumStops - 1));
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

	psLine = mpLayout->GetLine(uiRow);
	psStop = mpLayout->GetStop(psLine->uiFirstStop + psLine->uiNumStops - 1);
	if (IsRowWrapped(uiRow))
	{
		return psStop->x + psStop->iWidth;
	}
	return psStop->x;
}


//////////////////////////////////////////////////////////////////////////
//
// The row y (relative to the text area's top) is in.  Above the text it
// is the first, below it the last.
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::FindRowAt(int32 y)
{
	size				uiRow;
	size				uiNumRows;
	STextLayoutLine*	psLine;

	uiNumRows = mpLayout->NumLines();
	for (uiRow = 0; uiRow < uiNumRows - 1; uiRow++)
	{
		psLine = mpLayout->GetLine(uiRow);
		if (y < psLine->iTop + psLine->iHeight)
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

	psLine = mpLayout->GetLine(uiRow);
	uiEnd = psLine->uiFirstStop + psLine->uiNumStops;
	uiBest = psLine->uiFirstStop;
	iBestDistance = abs(mpLayout->GetStop(uiBest)->x - x);
	for (uiStop = uiBest + 1; uiStop < uiEnd; uiStop++)
	{
		iDistance = abs(mpLayout->GetStop(uiStop)->x - x);
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
	if (uiRow != mpLayout->GetStop(uiStop)->uiLine)
	{
		return GetRowRight(uiRow);
	}
	return mpLayout->GetStop(uiStop)->x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CTextArea::GetCaretX(void)
{
	return GetPositionX(muiCaret, mbCaretAtRowEnd);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextArea::GetCaretRow(void)
{
	return FindRow(muiCaret, mbCaretAtRowEnd);
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
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::IsSelected(void)
{
	return (muiAnchor != MAX_SIZE) && (muiAnchor != muiCaret);
}


//////////////////////////////////////////////////////////////////////////
//
// Moving without selecting clears the selection.  Selecting starts one at
// the caret if there isn't one.  Up and down keep the x they started at.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::MoveCaret(size uiPosition, bool bSelect, bool bRowEnd, bool bKeepUpDown)
{
	if (bSelect)
	{
		if (muiAnchor == MAX_SIZE)
		{
			muiAnchor = muiCaret;
		}
	}
	else
	{
		muiAnchor = MAX_SIZE;
	}

	muiCaret = uiPosition;
	mbCaretAtRowEnd = bRowEnd;
	if (!bKeepUpDown)
	{
		miUpDownX = -1;
	}
	UpdateMarks();
	ShowCaret();
}


//////////////////////////////////////////////////////////////////////////
//
// To the row above or below, or a page of rows, as near the x up and
// down started at as the row goes.  Up from the first row goes to the
// start of the text and down from the last to its end.  A page up or down
// from the first or last row doesn't move, otherwise the view scrolls as
// far as the caret moved so it stays where it was in the view.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::MoveUpDown(bool bDown, bool bPage, bool bSelect)
{
	size				uiRow;
	size				uiLastRow;
	STextLayoutLine*	psLine;
	int32				iPage;
	int32				iTop;
	size				uiPosition;
	bool				bRowEnd;

	if (!HasStops())
	{
		return;
	}

	if (miUpDownX < 0)
	{
		miUpDownX = GetCaretX();
	}
	uiRow = GetCaretRow();
	uiLastRow = mpLayout->NumLines() - 1;
	if ((!bDown && (uiRow == 0)) || (bDown && (uiRow == uiLastRow)))
	{
		if (!bPage)
		{
			MoveCaret(bDown ? mszText.Length() : 0, bSelect, false, true);
		}
		return;
	}

	if (bPage)
	{
		iPage = GetPageHeight();
		psLine = mpLayout->GetLine(uiRow);
		iTop = psLine->iTop;
		uiRow = FindRowAt(psLine->iTop + psLine->iHeight / 2 + (bDown ? iPage : -iPage));
		ScrollBy(mpLayout->GetLine(uiRow)->iTop - iTop);
	}
	else
	{
		uiRow = bDown ? uiRow + 1 : uiRow - 1;
	}

	uiPosition = FindPositionInRow(uiRow, miUpDownX, &bRowEnd);
	MoveCaret(uiPosition, bSelect, bRowEnd, true);
}


//////////////////////////////////////////////////////////////////////////
//
// To the start of a wrapped row, then to the first character on the line
// that isn't a space, then to the start of the line, and back.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::MoveHome(bool bSelect)
{
	size	uiLineStart;
	size	uiIndent;
	size	uiRowStart;

	uiLineStart = FindLineStart(muiCaret);
	if (HasStops())
	{
		uiRowStart = GetRowStart(GetCaretRow());
		if ((uiRowStart > uiLineStart) && ((uiRowStart != muiCaret) || mbCaretAtRowEnd))
		{
			MoveCaret(uiRowStart, bSelect);
			return;
		}
	}

	uiIndent = FindLineIndent(uiLineStart);
	MoveCaret(muiCaret == uiIndent ? uiLineStart : uiIndent, bSelect);
}


//////////////////////////////////////////////////////////////////////////
//
// To the end of a wrapped row, then to the end of the line.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::MoveEnd(bool bSelect)
{
	size	uiRowEnd;
	bool	bRowEnd;

	if (HasStops())
	{
		uiRowEnd = GetRowEnd(GetCaretRow(), &bRowEnd);
		if (bRowEnd && !((muiCaret == uiRowEnd) && mbCaretAtRowEnd))
		{
			MoveCaret(uiRowEnd, bSelect, true);
			return;
		}
	}
	MoveCaret(FindLineEnd(muiCaret), bSelect);
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
// szText goes in at uiPosition.  A span it is inside or at the end of
// grows around it, one after it moves.  So does the caret, and the
// selection's start, at or after uiPosition.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::InsertText(size uiPosition, const char* szText)
{
	size			uiLength;
	size			i;
	STextAreaSpan*	psSpan;

	uiLength = StrLen(szText);
	if (uiLength == 0)
	{
		return;
	}

	mszText.Insert(uiPosition, szText);
	for (i = 0; i < masSpans.NumElements(); i++)
	{
		psSpan = masSpans.Get(i);
		if (uiPosition <= psSpan->uiStart)
		{
			psSpan->uiStart += uiLength;
		}
		else if (uiPosition <= psSpan->uiStart + psSpan->uiLength)
		{
			psSpan->uiLength += uiLength;
		}
	}
	if (muiCaret >= uiPosition)
	{
		muiCaret += uiLength;
	}
	if ((muiAnchor != MAX_SIZE) && (muiAnchor >= uiPosition))
	{
		muiAnchor += uiLength;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Spans shrink by what is removed from them and go if nothing is left.
// The caret and the selection's start move with the text after.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::RemoveText(size uiStart, size uiEnd)
{
	size			uiLength;
	size			i;
	STextAreaSpan*	psSpan;
	size			uiSpanStart;
	size			uiSpanEnd;

	if (uiEnd <= uiStart)
	{
		return;
	}

	uiLength = uiEnd - uiStart;
	mszText.Remove(uiStart, uiEnd);
	i = masSpans.NumElements();
	while (i > 0)
	{
		i--;
		psSpan = masSpans.Get(i);
		uiSpanStart = psSpan->uiStart;
		uiSpanEnd = psSpan->uiStart + psSpan->uiLength;
		uiSpanStart = (uiSpanStart <= uiStart) ? uiSpanStart : ((uiSpanStart >= uiEnd) ? uiSpanStart - uiLength : uiStart);
		uiSpanEnd = (uiSpanEnd <= uiStart) ? uiSpanEnd : ((uiSpanEnd >= uiEnd) ? uiSpanEnd - uiLength : uiStart);
		if (uiSpanEnd == uiSpanStart)
		{
			masSpans.RemoveAt(i, true);
		}
		else
		{
			psSpan->uiStart = uiSpanStart;
			psSpan->uiLength = uiSpanEnd - uiSpanStart;
		}
	}

	if (muiCaret >= uiEnd)
	{
		muiCaret -= uiLength;
	}
	else if (muiCaret > uiStart)
	{
		muiCaret = uiStart;
	}
	if (muiAnchor != MAX_SIZE)
	{
		if (muiAnchor >= uiEnd)
		{
			muiAnchor -= uiLength;
		}
		else if (muiAnchor > uiStart)
		{
			muiAnchor = uiStart;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// szText replaces the selection, or is inserted at the caret.  The caret
// is after it and nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::ReplaceSelection(const char* szText)
{
	DeleteSelection();
	muiAnchor = MAX_SIZE;
	InsertText(muiCaret, szText);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if nothing was selected.
//
//////////////////////////////////////////////////////////////////////////
bool CTextArea::DeleteSelection(void)
{
	size	uiStart;
	size	uiEnd;

	if (!GetSelection(&uiStart, &uiEnd))
	{
		return false;
	}
	muiAnchor = MAX_SIZE;
	RemoveText(uiStart, uiEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Inserts a tab's worth of spaces at the start of each line from
// uiStart's to uiEnd's, but not uiEnd's if it is at its start.  A
// selection that started at the start of a line still does.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::IndentLines(size uiStart, size uiEnd)
{
	CArrayTextAreaStopStart	auiLineStarts;
	size					uiLine;
	size					i;
	CChars					szSpaces;
	bool					bAnchorFirst;
	bool					bAtLineStart;

	auiLineStarts.Init();
	uiLine = FindLineStart(uiStart);
	bAtLineStart = (uiLine == uiStart);
	for (;;)
	{
		*auiLineStarts.Add() = uiLine;
		uiLine = FindLineEnd(uiLine);
		if (uiLine >= mszText.Length())
		{
			break;
		}
		uiLine = NextPosition(uiLine);
		if (uiLine >= uiEnd)
		{
			break;
		}
	}

	szSpaces.Init();
	szSpaces.Append(' ', miTabWidth);
	bAnchorFirst = (muiAnchor != MAX_SIZE) && (muiAnchor < muiCaret);
	i = auiLineStarts.NumElements();
	while (i > 0)
	{
		i--;
		InsertText(*auiLineStarts.Get(i), szSpaces.Text());
	}
	szSpaces.Kill();
	auiLineStarts.Kill();

	if (bAtLineStart)
	{
		if (bAnchorFirst)
		{
			muiAnchor = uiStart;
		}
		else if (muiAnchor != MAX_SIZE)
		{
			muiCaret = uiStart;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Removes a tab, or up to a tab's worth of spaces, from the start of each
// line from uiStart's to uiEnd's, but not uiEnd's if it is at its start
// and after uiStart.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::OutdentLines(size uiStart, size uiEnd)
{
	CArrayTextAreaStopStart	auiLineStarts;
	size					uiLine;
	size					i;
	size					uiRemove;
	char*					szText;

	auiLineStarts.Init();
	uiLine = FindLineStart(uiStart);
	for (;;)
	{
		*auiLineStarts.Add() = uiLine;
		uiLine = FindLineEnd(uiLine);
		if (uiLine >= mszText.Length())
		{
			break;
		}
		uiLine = NextPosition(uiLine);
		if (uiLine >= uiEnd)
		{
			break;
		}
	}

	i = auiLineStarts.NumElements();
	while (i > 0)
	{
		i--;
		uiLine = *auiLineStarts.Get(i);
		szText = mszText.Text();
		uiRemove = 0;
		if (szText[uiLine] == '\t')
		{
			uiRemove = 1;
		}
		else
		{
			while ((uiRemove < (size)miTabWidth) && (szText[uiLine + uiRemove] == ' '))
			{
				uiRemove++;
			}
		}
		RemoveText(uiLine, uiLine + uiRemove);
	}
	auiLineStarts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The text is read and laid out again, the caret is kept in view and the
// window lays out again if the text's size changed.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::TextChanged(void)
{
	SInt32Vec2	sTextSize;

	sTextSize = msTextSize;
	mbCaretAtRowEnd = false;
	miUpDownX = -1;
	ReadText();
	LayoutText();
	if (((msTextSize.x != sTextSize.x) || (msTextSize.y != sTextSize.y)) && mpWindow.IsNotNull())
	{
		mpWindow->LayoutAgain();
	}
	ShowCaret();
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

	if ((mcActualBounds.GetWidth() <= 0) || !HasStops())
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
// The selection is a block on each row it is on, to the end of rows it
// goes past and a space further where they end at a line feed.  The
// caret is only drawn while the text area has focus.
//
//////////////////////////////////////////////////////////////////////////
void CTextArea::UpdateMarks(void)
{
	size				uiStart;
	size				uiEnd;
	size				uiRow;
	size				uiStartRow;
	size				uiEndRow;
	int32				iLeft;
	int32				iRight;
	STextLayoutLine*	psLine;
	CInt32Rect			cCaret;

	mpSelectionMap->Clear();
	mpCaretMap->Clear();
	if (!HasStops())
	{
		return;
	}

	if (GetSelection(&uiStart, &uiEnd))
	{
		uiStartRow = FindRow(uiStart, false);
		uiEndRow = FindRow(uiEnd, true);
		for (uiRow = uiStartRow; uiRow <= uiEndRow; uiRow++)
		{
			psLine = mpLayout->GetLine(uiRow);
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
				mpSelectionMap->AddRectangle(iLeft, psLine->iTop, iRight - iLeft, psLine->iHeight, msSelectionColour);
			}
		}
	}

	if (mbFocussed)
	{
		GetCaretBounds(&cCaret);
		mpCaretMap->AddRectangle(cCaret.GetLeft(), cCaret.GetTop(), cCaret.GetWidth(), cCaret.GetHeight(), msCaretColour);
	}
}


bool CTextArea::IsEditable(void) { return mbEditable; }
size CTextArea::GetCaretPosition(void) { return muiCaret; }
